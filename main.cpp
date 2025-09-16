#include <unordered_map>
#include <algorithm>
#include <iostream>

#include "imgui_lib.h"
#include "general.h"
#include "globals.h"
#include "wndproc.h"
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

    MultiSliderCallbackImGuiBind FlickMacroBind([](const int Value) {
        using Globals::FloatSliderFlags;
        using Globals::IntSliderFlags;
        using Globals::BooleanFlags;
        using Globals::ROBLOX_SENS_MULT;

        if (!BooleanFlags["Flick Macro"]) return;

        INPUT Input = { 0 };
        Input.type = INPUT_MOUSE;

        MOUSEINPUT* MouseInput = &Input.mi;
        MouseInput->dwFlags = MOUSEEVENTF_MOVE;

        const double Delay = 1.0 / static_cast<double>(IntSliderFlags["Flick Delay"]);
        LONG PixelsTravelled = 0;

        if (BooleanFlags["Human-like Flick"]) {
            const LONG TotalPixels = Value * ROBLOX_SENS_MULT * FloatSliderFlags["Flick Sensitivity"];
            const double Duration = 1.0 / static_cast<double>(IntSliderFlags["Flick Duration"]);

            auto DoPhase = [&](bool Reversed) {
                LONG Moved = 0;
                double ElapsedTime = 0.0;
                const double PhaseDuration = Duration / 2.0;

                if (PhaseDuration <= 0.0) {
                    LONG rem = TotalPixels;
                    MouseInput->dx = Reversed ? -rem : rem;
                    SendInput(1, &Input, sizeof(INPUT));
                    PixelsTravelled += rem;
                    return;
                }

                while (Moved < TotalPixels) {
                    ElapsedTime += ShortWait();

                    double t = ElapsedTime / PhaseDuration;
                    if (t < 0.0) t = 0.0;
                    if (t > 1.0) t = 1.0;

                    double Smoothed = 3.0 * t * t - 2.0 * t * t * t;

                    LONG Target = static_cast<LONG>(std::round(Smoothed * static_cast<double>(TotalPixels)));
                    LONG deltaPixels = Target - Moved;

                    if (deltaPixels > 0) {
                        MouseInput->dx = Reversed ? -deltaPixels : deltaPixels;
                        SendInput(1, &Input, sizeof(INPUT));
                        Moved += deltaPixels;
                        PixelsTravelled += deltaPixels;
                    }

                    if (t >= 1.0 && Moved < TotalPixels) {
                        LONG rem = TotalPixels - Moved;
                        if (rem > 0) {
                            MouseInput->dx = Reversed ? -rem : rem;
                            SendInput(1, &Input, sizeof(INPUT));
                            Moved += rem;
                            PixelsTravelled += rem;
                        }
                        break;
                    }
                }
            };

            DoPhase(false);
            Wait(Delay);
            DoPhase(true);
        } else {
            const LONG Pixels = Value * ROBLOX_SENS_MULT * FloatSliderFlags["Flick Sensitivity"];

            MouseInput->dx = Pixels;
            SendInput(1, &Input, sizeof(INPUT));

            Wait(Delay);

            MouseInput->dx = -Pixels;
            SendInput(1, &Input, sizeof(INPUT));
        }
    }, 90, -360, 360, "%d°");

    {
        Globals::IntSliderFlags["Flick Angle"] = 90;
        Globals::FloatSliderFlags["Flick Sensitivity"] = 1.0f;
        Globals::IntSliderFlags["Flick Delay"] = 60;
        Globals::IntSliderFlags["Flick Duration"] = 60;
    }

    ImVec2 ItemSpacing = ImGui::GetStyle().ItemSpacing;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ItemSpacing.x, ItemSpacing.x));

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        if (GetAsyncKeyState(VK_INSERT) & 1) {
            Globals::ImGuiShown = !Globals::ImGuiShown;
            ShowWindow(hwnd, Globals::ImGuiShown ? SW_SHOW : SW_HIDE);
        }

        if (Globals::ImGuiShown) {
            /*
                Some info:
                    SetNextWindowSize takes ImGuiStyle, a fixed width, and the amount of non-text-label elements
                        The fourth parameter is if to take the title-bar to calculation
                        The fifth parameter is given by an array of length 2 of [len, sum] where:
                            len is the amount of text-label elements
                            sum is the amount of line-wraps all text-label elements give
                        unfortunately, this is all hardcoded, but since this is just the gui section, doesn't really matter that much in the long-term.
            */

            using Globals::FloatSliderFlags;
            using Globals::IntSliderFlags;
            using Globals::BooleanFlags;

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            const ImGuiStyle Style = ImGui::GetStyle();

            constexpr auto WindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

            if (SetNextWindowSize(
                Style,
                200,
                3,
                false,
                GetTextElementsInfo(std::array{1})
            )&& ImGui::Begin("Input Macros", nullptr, WindowFlags)) {
                ImGui::Text("Mouse Actions");
                ImGui::Checkbox("Flick Macro", &BooleanFlags["Flick Macro"]);
                FlickMacroBind.Update();
                ImGui::Checkbox("Human-like Flick", &BooleanFlags["Human-like Flick"]);
                ImGui::End();
            }

            if (SetNextWindowSize(
                Style,
                310,
                0,
                false,
                GetTextElementsInfo(std::array{1, 1, 1, 2})
            )&& ImGui::Begin("Information", nullptr, WindowFlags)) {
                ImGui::PushTextWrapPos();
                ImGui::TextColored(ImVec4(1, 0.2, 0.2, 1), "INSERT KEY TO OPEN/CLOSE!");
                ImGui::Text("Thanks to TASability for inspiration!");
                ImGui::Text("Created by Anbubu (@anbubu on discord)");
                ImGui::TextColored(ImVec4(1, 0.3, 0.3, 1), "Make sure this program is acceptable for use on whatever game you're playing");
                ImGui::PopTextWrapPos();
                ImGui::End();
            }

            if (SetNextWindowSize(
                Style,
                144,
                1,
                true
            )&& ImGui::Begin("## Close Application Window", nullptr, WindowFlags | ImGuiWindowFlags_NoTitleBar)) {
                if (ImGui::Button("Close Application")) PostQuitMessage(0);
                ImGui::SetItemTooltip("Closes the application.");
                ImGui::End();
            }

            if (SetNextWindowSize(
                Style,
                293,
                3
            )&& ImGui::Begin("Global Settings", nullptr, WindowFlags)) {
                constexpr int AddSubtractIntSliderButtonWidth = 46;
                const int FramePaddingTotal = Style.FramePadding.x * 2;

                const float RegionWidth = ImGui::GetContentRegionAvail().x;
                const float FinalRegionWidth = RegionWidth - Style.ItemInnerSpacing.x;

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

            ImGui::Render();
            constexpr float clear_color[4] = { 0.00f, 0.00f, 0.00f, 0.00f };
            DirectX::g_pd3dDeviceContext->OMSetRenderTargets(1, &DirectX::g_mainRenderTargetView, NULL);
            DirectX::g_pd3dDeviceContext->ClearRenderTargetView(DirectX::g_mainRenderTargetView, clear_color);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }

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