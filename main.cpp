#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <chrono>

#include "imgui_lib.h"
#include "general.h"
#include "globals.h"
#include "wndproc.h"
#include "macros.h"
#include "tasks.h"
#include "bind.h"
#include "init.h"

#include <tchar.h>

#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#pragma comment(lib, "d3d11.lib")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    namespace DirectX = Globals::DirectX;
    using Globals::NotifyIconData;
    using Globals::FloatSliderFlags;
    using Globals::IntSliderFlags;
    using Globals::BooleanFlags;

    WNDCLASSEX wc = InitialiseWindow(hInstance);
    RegisterClassEx(&wc);

    Globals::g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    if (!Globals::g_hHook) {
        MessageBox(NULL, _T("Failed to install keyboard hook!"), _T("Error"), MB_ICONERROR);
    }

    const int nWidth = GetSystemMetrics(SM_CXSCREEN);
    const int nHeight = GetSystemMetrics(SM_CYSCREEN);

    HWND hwnd = CreateWindowEx(
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

    {
        BooleanFlags["Flick Back Macro"] = false;
        BooleanFlags["Human-like Flick"] = false;
        BooleanFlags["Snap Back Macro"] = false;
        BooleanFlags["Spam Back Macro"] = false;

        FloatSliderFlags["Flick Sensitivity"] = 1.0f;

        IntSliderFlags["Flick Delay"] = 60;
        IntSliderFlags["Flick Duration"] = 60;
    }

    ImVec2 ItemSpacing = ImGui::GetStyle().ItemSpacing;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ItemSpacing.x, ItemSpacing.x));

    auto Time = std::chrono::high_resolution_clock::now();

    Globals::RenderStepped.connect([](const auto Delta) {
        std::cerr << "Total Time Rendering: " << std::to_string(Delta.count() * 1000) << "ms" << std::endl;
    });

    while (msg.message != WM_QUIT) {

#ifdef _DEBUG
        const auto t1 = std::chrono::high_resolution_clock::now();
#endif

        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

#ifdef _DEBUG
        const auto t2 = std::chrono::high_resolution_clock::now();
#endif

        // INSERT to close and open GUI
        if (GetAsyncKeyState(Globals::OPEN_CLOSE_KEY) & 1) {
            Globals::ImGuiShown = !Globals::ImGuiShown;
            ShowWindow(hwnd, Globals::ImGuiShown ? SW_SHOW : SW_HIDE);
        }

#ifdef _DEBUG
        const auto t3 = std::chrono::high_resolution_clock::now();
#endif

        // RenderStepped Handler
        const auto Now = std::chrono::high_resolution_clock::now();
        Globals::RenderStepped.fire(Now - Time);
        Time = Now;

#ifdef _DEBUG
        const auto t4 = std::chrono::high_resolution_clock::now();
#endif

        if (!Globals::ImGuiShown) {
            DirectX::g_pSwapChain->Present(1, 0);

            std::cout << "Messages: " << std::chrono::duration<float, std::milli>(t2-t1).count() << "ms, "
                      << "Gui Close Handler: " << std::chrono::duration<float, std::milli>(t3-t2).count() << "ms, "
                      << "RenderStepped Handler: " << std::chrono::duration<float, std::milli>(t4-t3).count() << "ms, ";

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

#ifdef _DEBUG
        const auto t5 = std::chrono::high_resolution_clock::now();
#endif

        const ImGuiStyle Style = ImGui::GetStyle();

#ifdef _DEBUG
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
                static bool* const FlickBackMacro = &BooleanFlags["Flick Back Macro"];
                static bool* const SnapBackMacro = &BooleanFlags["Snap Back Macro"];
                static bool* const SpamBackMacro = &BooleanFlags["Spam Back Macro"];

                static MultiSliderCallbackImGuiBind FlickBackMacroBind([](const int Value) {
                    if (BooleanFlags["Flick Back Macro"]) MouseFlick(Value);
                }, 90, -360, 360, "%d°");

                static MultiSliderCallbackImGuiBind SnapFlickMacroBind([](const int Value) {
                    if (BooleanFlags["Snap Flick Macro"]) MouseFlick(Value, true);
                }, 90, -360, 360, "%d°");

                static MultiSliderCallbackImGuiBind SpamFlickMacroBind([](const int Value) {
                    if (BooleanFlags["Spam Flick Macro"]) MouseFlick(Value);
                    Wait(1.0 / 60.0);
                }, 90, -360, 360, "%d°", BindMode::Hold);

                ImGui::Text("Mouse Actions");
                ImGui::Checkbox("Flick Back Macro", FlickBackMacro); FlickBackMacroBind.Update();
                ImGui::Checkbox("Snap Flick Macro", SnapBackMacro); SnapFlickMacroBind.Update();
                ImGui::Checkbox("Spam Flick Macro", SpamBackMacro); SpamFlickMacroBind.Update();
                ImGui::End();
            }
        }

#ifdef _DEBUG
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

#ifdef _DEBUG
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

#ifdef _DEBUG
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
                constexpr int AddSubtractIntSliderButtonWidth = 46;

                static const float RegionWidth = ImGui::GetContentRegionAvail().x;
                static const float FinalRegionWidth = RegionWidth - Style.ItemInnerSpacing.x;
                
                {
                    constexpr const char* ElementName = "Human-like Flick";

                    static bool* const HumanLikeFlick = &BooleanFlags[ElementName];
                    ImGui::Checkbox(ElementName, HumanLikeFlick);
                }

                {
                    constexpr const char* ElementName = "Flick Sensitivity";

                    static float* const FlickSensitivity = &FloatSliderFlags[ElementName];
                    *FlickSensitivity = std::clamp(*FlickSensitivity, 0.0f, 10.0f);

                    static const float ItemWidth = FinalRegionWidth - ImGui::CalcTextSize(ElementName).x;
                    ImGui::SetNextItemWidth(ItemWidth);
                    ImGui::SliderFloat(ElementName, FlickSensitivity, 0.0f, 10.0f, "%.2f");
                    ImGui::SetItemTooltip("Set this slider to your in-game roblox sensitivity.");
                }

                {
                    constexpr const char* ElementName = "Flick Delay";

                    static int* const FlickDelay = &IntSliderFlags[ElementName];
                    *FlickDelay = std::clamp(*FlickDelay, 1, 240);

                    static const float ItemWidth = FinalRegionWidth - ImGui::CalcTextSize(ElementName).x;
                    ImGui::SetNextItemWidth(ItemWidth);
                    ImGui::SliderInt(ElementName, FlickDelay, 1, 240, "1 / %ds");
                    ImGui::SetItemTooltip("The delay in between flicking.");
                }

                {
                    constexpr const char* ElementName = "Flick Duration";

                    static int* const FlickDuration = &IntSliderFlags[ElementName];
                    *FlickDuration = std::clamp(*FlickDuration, 1, 240);

                    static const float ItemWidth = FinalRegionWidth - ImGui::CalcTextSize(ElementName).x;
                    ImGui::SetNextItemWidth(ItemWidth);
                    ImGui::SliderInt(ElementName, FlickDuration, 1, 240, "1 / %ds");
                    ImGui::SetItemTooltip("The duration of the human-like flicks. Does nothing if \"Human-like Flick\" is off.");
                }

                ImGui::End();
            }
        }

#ifdef _DEBUG
        const auto t10 = std::chrono::high_resolution_clock::now();
#endif

        ImGui::Render();

#ifdef _DEBUG
        const auto t11 = std::chrono::high_resolution_clock::now();
#endif

        constexpr float ClearColor[4] = { 0.00f, 0.00f, 0.00f, 0.00f };
        DirectX::g_pd3dDeviceContext->OMSetRenderTargets(1, &DirectX::g_mainRenderTargetView, NULL);
        DirectX::g_pd3dDeviceContext->ClearRenderTargetView(DirectX::g_mainRenderTargetView, ClearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

#ifdef _DEBUG
        const auto t12 = std::chrono::high_resolution_clock::now();
#endif

        DirectX::g_pSwapChain->Present(0, 0);

#ifdef _DEBUG
        std::cout << "Messages: " << std::chrono::duration<float, std::milli>(t2 - t1).count() << "ms" << std::endl
                  << "Gui Close Handler: " << std::chrono::duration<float, std::milli>(t3 - t2).count() << "ms" << std::endl
                  << "RenderStepped Handler: " << std::chrono::duration<float, std::milli>(t4 - t3).count() << "ms" << std::endl
                  << "NewFrame: " << std::chrono::duration<float, std::milli>(t5 - t4).count() << "ms" << std::endl
                  << "GetStyle: " << std::chrono::duration<float, std::milli>(t6 - t5).count() << "ms" << std::endl
                  << "Window 1: " << std::chrono::duration<float, std::milli>(t7 - t6).count() << "ms" << std::endl
                  << "Window 2: " << std::chrono::duration<float, std::milli>(t8 - t7).count() << "ms" << std::endl
                  << "Window 3: " << std::chrono::duration<float, std::milli>(t9 - t8).count() << "ms" << std::endl
                  << "Window 4: " << std::chrono::duration<float, std::milli>(t10 - t9).count() << "ms" << std::endl
                  << "ImGui::Render(): " << std::chrono::duration<float, std::milli>(t11 - t10).count() << "ms" << std::endl
                  << "RenderTargets: " << std::chrono::duration<float, std::milli>(t12 - t11).count() << "ms" << std::endl
                  << "g_pSwapChain->Present(0, 0): " << std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - t12).count() << "ms" << std::endl;
#endif
    }

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