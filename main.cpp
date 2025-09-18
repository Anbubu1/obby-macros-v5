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

    std::cerr << ToUpper("hello woRLD!") << std::endl;

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

    MultiSliderCallbackImGuiBind FlickBackMacroBind([](const int Value) {
        if (BooleanFlags["Flick Back Macro"]) MouseFlick(Value);
    }, 90, -360, 360, "%d°");

    MultiSliderCallbackImGuiBind SnapFlickMacroBind([](const int Value) {
        if (BooleanFlags["Snap Flick Macro"]) MouseFlick(Value, true);
    }, 90, -360, 360, "%d°");

    MultiSliderCallbackImGuiBind SpamFlickMacroBind([](const int Value) {
        if (BooleanFlags["Spam Flick Macro"]) MouseFlick(Value);
        Wait(1.0 / 60.0);
    }, 90, -360, 360, "%d°", BindMode::Hold);

    {
        IntSliderFlags["Flick Angle"] = 90;
        FloatSliderFlags["Flick Sensitivity"] = 1.0f;
        IntSliderFlags["Flick Delay"] = 60;
        IntSliderFlags["Flick Duration"] = 60;
    }

    ImVec2 ItemSpacing = ImGui::GetStyle().ItemSpacing;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ItemSpacing.x, ItemSpacing.x));

    auto Time = std::chrono::high_resolution_clock::now();

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        // INSERT to close and open GUI
        if (GetAsyncKeyState(VK_INSERT) & 1) {
            Globals::ImGuiShown = !Globals::ImGuiShown;
            ShowWindow(hwnd, Globals::ImGuiShown ? SW_SHOW : SW_HIDE);
        }

        // RenderStepped Handler
        const auto Now = std::chrono::high_resolution_clock::now();
        Globals::RenderStepped.fire(Now - Time);
        Time = Now;

        if (!Globals::ImGuiShown) {
            DirectX::g_pSwapChain->Present(1, 0);
            continue;
        }

        /*
            Some info:
                SetNextWindowSize takes ImGuiStyle, a fixed width, and the amount of non-text-label elements
                    The fourth parameter is if to take the title-bar to calculation
                    The fifth parameter is given by an array of length 2 of [len, sum] where:
                        len is the amount of text-label elements
                        sum is the amount of line-wraps all text-label elements give
                    unfortunately, this is all hardcoded, but since this is just the gui section, doesn't really matter that much in the long-term.
        */

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        const ImGuiStyle Style = ImGui::GetStyle();

        constexpr auto WindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

        {
            constexpr int WindowWidth = 225;
            constexpr int NonTextElements = 3;
            constexpr int IgnoreTitleBar = false;
            constexpr auto TextElementsInfo = GetTextElementsInfo(std::array{1});
            if (SetNextWindowSize(
                Style,
                WindowWidth,
                NonTextElements,
                IgnoreTitleBar,
                TextElementsInfo
            )&& ImGui::Begin("Input Macros", nullptr, WindowFlags)) {
                ImGui::Text("Mouse Actions");
                ImGui::Checkbox("Flick Back Macro", &BooleanFlags["Flick Back Macro"]);
                FlickBackMacroBind.Update();
                ImGui::Checkbox("Snap Flick Macro", &BooleanFlags["Snap Flick Macro"]);
                SnapFlickMacroBind.Update();
                ImGui::Checkbox("Spam Flick Macro", &BooleanFlags["Spam Flick Macro"]);
                SpamFlickMacroBind.Update();
                ImGui::End();
            }
        }

        {
            constexpr int WindowWidth = 310;
            constexpr int NonTextElements = 0;
            constexpr int IgnoreTitleBar = false;
            // e.g. a line of text, then another line of text, and another line of text, followed by 2 lines of text
            constexpr auto TextElementsInfo = GetTextElementsInfo(std::array{1, 1, 1, 2});
            if (SetNextWindowSize(
                Style,
                WindowWidth,
                NonTextElements,
                IgnoreTitleBar,
                TextElementsInfo
            )&& ImGui::Begin("Information", nullptr, WindowFlags)) {
                ImGui::PushTextWrapPos();
                ImGui::TextColored(ImVec4(1, 0.2, 0.2, 1), "INSERT KEY TO OPEN/CLOSE!");
                ImGui::Text("Thanks to TASability for inspiration!");
                ImGui::Text("Created by Anbubu (@anbubu on discord)");
                ImGui::TextColored(ImVec4(1, 0.3, 0.3, 1), "Make sure this program is acceptable for use on whatever game you're playing");
                ImGui::PopTextWrapPos();
                ImGui::End();
            }
        }

        {
            constexpr int WindowWidth = 144;
            constexpr int NonTextElements = 1;
            constexpr int IgnoreTitleBar = true;
            if (SetNextWindowSize(
                Style,
                WindowWidth,
                NonTextElements,
                IgnoreTitleBar
            )&& ImGui::Begin("## Close Application Window", nullptr, WindowFlags | ImGuiWindowFlags_NoTitleBar)) {
                if (ImGui::Button("Close Application")) PostQuitMessage(0);
                ImGui::SetItemTooltip("Closes the application.");
                ImGui::End();
            }
        }

        {
            constexpr int WindowWidth = 250;
            constexpr int NonTextElements = 4;
            if (SetNextWindowSize(
                Style,
                WindowWidth,
                NonTextElements
            )&& ImGui::Begin("Global Settings", nullptr, WindowFlags)) {
                constexpr int AddSubtractIntSliderButtonWidth = 46;
                const int FramePaddingTotal = Style.FramePadding.x * 2;

                const float RegionWidth = ImGui::GetContentRegionAvail().x;
                const float FinalRegionWidth = RegionWidth - Style.ItemInnerSpacing.x;

                ImGui::Checkbox("Human-like Flick", &BooleanFlags["Human-like Flick"]);

                {
                    FloatSliderFlags["Flick Sensitivity"] = std::clamp(FloatSliderFlags["Flick Sensitivity"], 0.0f, 10.0f);

                    ImGui::SetNextItemWidth(FinalRegionWidth - ImGui::CalcTextSize("Flick Sensitivity").x);
                    ImGui::SliderFloat("Flick Sensitivity", &FloatSliderFlags["Flick Sensitivity"], 0.0f, 10.0f, "%.2f");
                    ImGui::SetItemTooltip("Set this slider to your in-game roblox sensitivity.");
                }

                {
                    IntSliderFlags["Flick Delay"] = std::clamp(IntSliderFlags["Flick Delay"], 1, 240);

                    ImGui::SetNextItemWidth(FinalRegionWidth - ImGui::CalcTextSize("Flick Delay").x);
                    ImGui::SliderInt("Flick Delay", &IntSliderFlags["Flick Delay"], 1, 240, "1 / %ds");
                    ImGui::SetItemTooltip("The delay in between flicking.");
                }

                {
                    IntSliderFlags["Flick Duration"] = std::clamp(IntSliderFlags["Flick Duration"], 1, 240);

                    ImGui::SetNextItemWidth(FinalRegionWidth - ImGui::CalcTextSize("Flick Duration").x);
                    ImGui::SliderInt("Flick Duration", &IntSliderFlags["Flick Duration"], 1, 240, "1 / %ds");
                    ImGui::SetItemTooltip("The duration of the human-like flicks. Does nothing if \"Human-like Flick\" is off.");
                }

                ImGui::End();
            }
        }

        ImGui::Render();
        constexpr float ClearColor[4] = { 0.00f, 0.00f, 0.00f, 0.00f };
        DirectX::g_pd3dDeviceContext->OMSetRenderTargets(1, &DirectX::g_mainRenderTargetView, NULL);
        DirectX::g_pd3dDeviceContext->ClearRenderTargetView(DirectX::g_mainRenderTargetView, ClearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        DirectX::g_pSwapChain->Present(1, 0);
    }

    if (Globals::g_hHook) {
        UnhookWindowsHookEx(Globals::g_hHook);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    Shell_NotifyIcon(NIM_DELETE, &NotifyIconData);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}