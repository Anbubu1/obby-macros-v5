#undef __RenderDebug

#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <chrono>
#include <format>

#include <conversion.hpp>
#include <imgui_lib.hpp>
#include <elements.hpp>
#include <general.hpp>
#include <globals.hpp>
#include <wndproc.hpp>
#include <macros.hpp>
#include <tasks.hpp>
#include <bind.hpp>
#include <init.hpp>

#include <tchar.h>

#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#pragma comment(lib, "d3d11.lib")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    namespace DirectX = Globals::DirectX;
    using Globals::NotifyIconData;
    using Globals::FloatSliderFlags;
    using Globals::IntSliderFlags;
    using Globals::BooleanFlags;

    { // Create Configuration Folder
        namespace filesystem = std::filesystem;

        wchar_t buffer[MAX_PATH];
        if (GetEnvironmentVariableW(L"LOCALAPPDATA", buffer, MAX_PATH) == 0)
            throw std::runtime_error("Failed to get \%localappdata\%!");

        Globals::ConfigPath = filesystem::path(buffer) / "Obby-Macros" / "configs" / "default.json";

        if (!filesystem::exists(Globals::ConfigPath)
         && !filesystem::create_directories(Globals::ConfigPath.parent_path()))
            throw std::runtime_error("Failed to create ./config directory!");

        if (!filesystem::exists(Globals::ConfigPath)) {
            std::ofstream File(Globals::ConfigPath);
            if (!File)
                throw std::runtime_error("Failed to create default.json!");
            File.close();
        } else {
            std::ifstream File(Globals::ConfigPath);
            std::stringstream Buffer;
            Buffer << File.rdbuf();
            std::string Contents = Buffer.str();
            if (Contents.empty())
                Globals::JSONConfig = Globals::GetDefaultConfig();
            else {
                Globals::JSONConfig = nlohmann::json::parse(Contents);
                const std::string JSONDump = Globals::JSONConfig.dump(4);
                std::printf(JSONDump.c_str());
                if (!Globals::JSONConfig.contains("BooleanFlags"))
                    Globals::JSONConfig = Globals::GetDefaultConfig();
            }
        }
    }

    WNDCLASSEX wc = InitialiseWindow(hInstance);
    RegisterClassEx(&wc);

    Globals::g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    if (!Globals::g_hHook) {
        MessageBox(NULL, _T("Failed to install keyboard hook!"), _T("Error"), MB_ICONERROR);
    }

    const int nWidth = GetSystemMetrics(SM_CXSCREEN);
    const int nHeight = GetSystemMetrics(SM_CYSCREEN);

    const HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        wc.lpszClassName,
        _T("Dear ImGui Win32 + DirectX11 Example"),
        WS_POPUP,
        0, 0, nWidth, nHeight,
        NULL, NULL, wc.hInstance, NULL
    );

    Globals::MainWindow = hwnd;
    EnableBlur(hwnd);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    NotifyIconData.cbSize = sizeof(NotifyIconData);
    NotifyIconData.hWnd = hwnd;
    NotifyIconData.uID = 1;
    NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    NotifyIconData.uCallbackMessage = WM_APP + 1;
    NotifyIconData.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    lstrcpy(NotifyIconData.szTip, TEXT("Obby Macros V5 C++"));

    Shell_NotifyIcon(NIM_ADD, &NotifyIconData);

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(DirectX::g_pd3dDevice, DirectX::g_pd3dDeviceContext);

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    const ImVec2 ItemSpacing = ImGui::GetStyle().ItemSpacing;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ItemSpacing.x, ItemSpacing.x));

    auto Time = std::chrono::high_resolution_clock::now();

#ifdef __RenderDebug
    Globals::RenderStepped.connect([](const auto Delta) {
        std::cerr << "Total Time Rendering: " << std::to_string(Delta.count() * 1000) << "ms" << std::endl;
    });
#endif

    while (msg.message != WM_QUIT) {

#ifdef __RenderDebug
        const auto t1 = std::chrono::high_resolution_clock::now();
#endif

        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

#ifdef __RenderDebug
        const auto t2 = std::chrono::high_resolution_clock::now();
#endif

        // INSERT to close and open GUI
        if (GetAsyncKeyState(Globals::OPEN_CLOSE_KEY) & 1) {
            Globals::ImGuiShown = !Globals::ImGuiShown;
            ShowWindow(hwnd, Globals::ImGuiShown ? SW_SHOW : SW_HIDE);
        }

#ifdef __RenderDebug
        const auto t3 = std::chrono::high_resolution_clock::now();
#endif

        // RenderStepped Handler
        const auto Now = std::chrono::high_resolution_clock::now();
        Globals::RenderStepped.fire(Now - Time);
        Time = Now;

#ifdef __RenderDebug
        const auto t4 = std::chrono::high_resolution_clock::now();
#endif

        if (!Globals::ImGuiShown) {
            DirectX::g_pSwapChain->Present(1, 0);

#ifdef __RenderDebug
            std::cout << "Messages: " << std::chrono::duration<float, std::milli>(t2-t1).count() << "ms, "
                      << "Gui Close Handler: " << std::chrono::duration<float, std::milli>(t3-t2).count() << "ms, "
                      << "RenderStepped Handler: " << std::chrono::duration<float, std::milli>(t4-t3).count() << "ms, ";
#endif

            continue;
        }

        /*
            Some info:
                GetNextWindowSize takes ImGuiStyle, a fixed width, and the amount of non-text-label elements
                    The fourth parameter is if to take the title-bar to calculation
                    The fifth parameter is given by an array of length 2 of [len, sum] where:
                        len is the amount of text-label elements
                        sum is the amount of line-wraps all text-label elements give
                    unfortunately, this is all hardcoded, but since this is just the gui section, doesn't really matter that much in the long-term.
        */

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

#ifdef __RenderDebug
        const auto t5 = std::chrono::high_resolution_clock::now();
#endif

        const ImGuiStyle Style = ImGui::GetStyle();

#ifdef __RenderDebug
        const auto t6 = std::chrono::high_resolution_clock::now();
#endif

        constexpr auto WindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

        {
            constexpr int WindowWidth = 225;
            constexpr int NonTextElements = 3;
            constexpr int IgnoreTitleBar = false;
            constexpr auto TextElementsInfo = GetTextElementsInfo(std::array{1});

            static const ImVec2 WindowSize = GetNextWindowSize(
                Style,
                WindowWidth,
                NonTextElements,
                IgnoreTitleBar,
                TextElementsInfo
            );

            if (SetNextWindowSize(WindowSize) && ImGui::Begin("Input Macros", nullptr, WindowFlags)) {
                ImGui::Text("Mouse Actions");

                {
                    constexpr const char* ElementName = "Flick Back Macro";
                    static Checkbox Checkbox(ElementName, false, std::make_unique<MultiSliderCallbackImGuiBind>([](const int Value) {
                        static std::atomic<bool>* const Flag = &BooleanFlags[ElementName];
                        if (Flag->load()) MouseFlick(Value);
                    }, 90, -360, 360, "%d°"));
                    Checkbox.Update();
                }

                {
                    constexpr const char* ElementName = "Flick Macro";
                    static Checkbox Checkbox(ElementName, false, std::make_unique<MultiSliderCallbackImGuiBind>([](const int Value) {
                        static std::atomic<bool>* const Flag = &BooleanFlags[ElementName];
                        if (Flag->load()) MouseFlick(Value, true);
                    }, 90, -360, 360, "%d°"));
                    Checkbox.Update();
                }

                {
                    constexpr const char* ElementName = "Spam Flick Macro";
                    static Checkbox Checkbox(ElementName, false, std::make_unique<MultiSliderCallbackImGuiBind>([](const int Value) {
                        static std::atomic<bool>* const Flag = &BooleanFlags[ElementName];
                        if (Flag->load()) MouseFlick(Value);
                        Wait(1.0 / 60.0);
                    }, 90, -360, 360, "%d°", BindMode::Hold));
                    Checkbox.Update();
                }

                ImGui::End();
            }
        }

#ifdef __RenderDebug
        const auto t7 = std::chrono::high_resolution_clock::now();
#endif

        {
            constexpr int WindowWidth = 310;
            constexpr int NonTextElements = 0;
            constexpr int IgnoreTitleBar = false;
            // e.g. a line of text, then another line of text, and another line of text, followed by 2 lines of text
            constexpr auto TextElementsInfo = GetTextElementsInfo(std::array{1, 1, 1, 2, 1});

            static const ImVec2 WindowSize = GetNextWindowSize(
                Style,
                WindowWidth,
                NonTextElements,
                IgnoreTitleBar,
                TextElementsInfo
            );

            if (SetNextWindowSize(WindowSize) && ImGui::Begin("Information", nullptr, WindowFlags)) {
                ImGui::PushTextWrapPos();
                ImGui::TextColored(ImVec4(1, 0.2, 0.2, 1), "INSERT KEY TO OPEN/CLOSE!");
                ImGui::Text("Thanks to TASability for inspiration!");
                ImGui::Text("Created by Anbubu (@anbubu on discord)");
                ImGui::TextColored(ImVec4(1, 0.3, 0.3, 1), "Make sure this program is acceptable for use on whatever game you're playing");
                ImGui::Text("The app is in the hidden icon tray.");
                ImGui::PopTextWrapPos();
                ImGui::End();
            }
        }

#ifdef __RenderDebug
        const auto t8 = std::chrono::high_resolution_clock::now();
#endif

        {
            constexpr int WindowWidth = 144;
            constexpr int NonTextElements = 1;
            constexpr int IgnoreTitleBar = true;

            static const ImVec2 WindowSize = GetNextWindowSize(
                Style,
                WindowWidth,
                NonTextElements,
                IgnoreTitleBar
            );

            if (SetNextWindowSize(WindowSize) && ImGui::Begin("## Close Application Window", nullptr, WindowFlags | ImGuiWindowFlags_NoTitleBar)) {
                if (ImGui::Button("Close Application")) PostQuitMessage(0);
                ImGui::SetItemTooltip("Closes the application.");
                ImGui::End();
            }
        }

#ifdef __RenderDebug
        const auto t9 = std::chrono::high_resolution_clock::now();
#endif

        {
            constexpr int WindowWidth = 250;
            constexpr int NonTextElements = 4;

            static const ImVec2 WindowSize = GetNextWindowSize(
                Style,
                WindowWidth,
                NonTextElements
            );

            if (SetNextWindowSize(WindowSize) && ImGui::Begin("Global Settings", nullptr, WindowFlags)) {
                static const float RegionWidth = ImGui::GetContentRegionAvail().x;
                static const float FinalRegionWidth = RegionWidth - Style.ItemInnerSpacing.x;
                
                {
                    static Checkbox Checkbox("Human-like Flicking");
                    Checkbox.Update();
                }

                {
                    constexpr const char* ElementName = "Flick Sensitivity";
                    static Slider<float> Slider(ElementName, 1.0f, 0.0f, 10.0f, "%.2f");

                    static const float ItemWidth = FinalRegionWidth - ImGui::CalcTextSize(ElementName).x;
                    ImGui::SetNextItemWidth(ItemWidth);
                    Slider.Update();
                    ImGui::SetItemTooltip("Set this slider to your in-game roblox sensitivity.");
                }

                {
                    constexpr const char* ElementName = "Flick Delay";
                    static Slider<int> Slider(ElementName, 60, 1, 240, "1 / %ds");

                    static const float ItemWidth = FinalRegionWidth - ImGui::CalcTextSize(ElementName).x;
                    ImGui::SetNextItemWidth(ItemWidth);
                    Slider.Update();
                    ImGui::SetItemTooltip("The delay in between flicking.");
                }

                {
                    constexpr const char* ElementName = "Flick Duration";
                    static Slider<int> Slider(ElementName, 60, 1, 240, "1 / %ds");

                    static const float ItemWidth = FinalRegionWidth - ImGui::CalcTextSize(ElementName).x;
                    ImGui::SetNextItemWidth(ItemWidth);
                    Slider.Update();
                    ImGui::SetItemTooltip("The duration of the human-like flicks. Does nothing if \"Human-like Flicking\" is off.");
                }

                ImGui::End();
            }
        }

#ifdef __RenderDebug
        const auto t10 = std::chrono::high_resolution_clock::now();
#endif

        ImGui::Render();

#ifdef __RenderDebug
        const auto t11 = std::chrono::high_resolution_clock::now();
#endif

        constexpr float ClearColor[4] = { 0.00f, 0.00f, 0.00f, 0.00f };
        DirectX::g_pd3dDeviceContext->OMSetRenderTargets(1, &DirectX::g_mainRenderTargetView, NULL);
        DirectX::g_pd3dDeviceContext->ClearRenderTargetView(DirectX::g_mainRenderTargetView, ClearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

#ifdef __RenderDebug
        const auto t12 = std::chrono::high_resolution_clock::now();
#endif

        DirectX::g_pSwapChain->Present(0, 0);

#ifdef __RenderDebug
        const auto t13 = std::chrono::high_resolution_clock::now();
        std::cout << "Messages: "                    << std::chrono::duration<float, std::milli>(t2 - t1).count()   << "ms" << std::endl
                  << "Gui Close Handler: "           << std::chrono::duration<float, std::milli>(t3 - t2).count()   << "ms" << std::endl
                  << "RenderStepped Handler: "       << std::chrono::duration<float, std::milli>(t4 - t3).count()   << "ms" << std::endl
                  << "NewFrame: "                    << std::chrono::duration<float, std::milli>(t5 - t4).count()   << "ms" << std::endl
                  << "GetStyle: "                    << std::chrono::duration<float, std::milli>(t6 - t5).count()   << "ms" << std::endl
                  << "Window 1: "                    << std::chrono::duration<float, std::milli>(t7 - t6).count()   << "ms" << std::endl
                  << "Window 2: "                    << std::chrono::duration<float, std::milli>(t8 - t7).count()   << "ms" << std::endl
                  << "Window 3: "                    << std::chrono::duration<float, std::milli>(t9 - t8).count()   << "ms" << std::endl
                  << "Window 4: "                    << std::chrono::duration<float, std::milli>(t10 - t9).count()  << "ms" << std::endl
                  << "ImGui::Render(): "             << std::chrono::duration<float, std::milli>(t11 - t10).count() << "ms" << std::endl
                  << "RenderTargets: "               << std::chrono::duration<float, std::milli>(t12 - t11).count() << "ms" << std::endl
                  << "g_pSwapChain->Present(0, 0): " << std::chrono::duration<float, std::milli>(t13 - t12).count() << "ms" << std::endl;
#endif
    }

    std::ofstream ConfigFile(Globals::ConfigPath);
    const std::string JSONDump = Globals::JSONConfig.dump();
    ConfigFile.write(JSONDump.c_str(), JSONDump.length());
    ConfigFile.close();

    if (Globals::g_hHook) {
        UnhookWindowsHookEx(Globals::g_hHook);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    Shell_NotifyIcon(NIM_DELETE, &NotifyIconData);

    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CleanupDeviceD3D();
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}