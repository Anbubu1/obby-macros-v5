#include <windows_lib.hpp>

#include <imgui_lib.hpp>
#include <globals.hpp>
#include <tasks.hpp>
#include <bind.hpp>

#include <imgui.h>

#include <tchar.h>

#include <windows.h>

LRESULT CALLBACK KeyboardProc(const int nCode, const WPARAM wParam, const LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

        if ((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) && p->vkCode != Globals::OPEN_CLOSE_KEY) {
            Binds::KeyPressed.fire(p->vkCode);
        }

        if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP && p->vkCode != Globals::OPEN_CLOSE_KEY) {
            Binds::KeyReleased.fire(p->vkCode);
        }
    }

    return CallNextHookEx(Globals::g_hHook, nCode, wParam, lParam);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return 1;

    switch (msg) {
        case WM_DISPLAYCHANGE: {
            const int nWidth = LOWORD(lParam);
            const int nHeight = HIWORD(lParam);

            if (Globals::DirectX::g_pd3dDevice != NULL) {
                CleanupRenderTarget();
                Globals::DirectX::g_pSwapChain->ResizeBuffers(0, nWidth, nHeight, DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
            }

            break;
        }

        case WM_NCHITTEST: {
            if (!Globals::ImGuiShown)
                return HTTRANSPARENT;

            return DefWindowProc(hWnd, msg, wParam, lParam);
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_SIZE: {
            if (Globals::DirectX::g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {
                CleanupRenderTarget();
                Globals::DirectX::g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
            }
            return 0;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }

        case WM_SYSCOMMAND: {
            if ((wParam & 0xfff0) == SC_KEYMENU)
                return 0;
            break;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case 1:
                    Globals::ImGuiShown = !Globals::ImGuiShown;
                    ShowWindow(hWnd, Globals::ImGuiShown ? SW_SHOW : SW_HIDE);
                    break;
                case 2:
                    PostQuitMessage(0);
                    break;
            }
            return 0;
        }
        
        case WM_APP + 1: {
            switch (lParam) {
                case WM_RBUTTONUP: {
                    POINT pt;
                    GetCursorPos(&pt);

                    HMENU hMenu = CreatePopupMenu();
                    AppendMenu(hMenu, MF_STRING, 1, TEXT("Show/Hide"));
                    AppendMenu(hMenu, MF_STRING, 2, TEXT("Exit"));

                    SetForegroundWindow(hWnd);

                    TrackPopupMenu(
                        hMenu,
                        TPM_BOTTOMALIGN | TPM_LEFTALIGN,
                        pt.x, pt.y,
                        0, hWnd, NULL
                    );

                    DestroyMenu(hMenu);
                }

                case WM_LBUTTONDBLCLK: {
                    if (Globals::ImGuiShown) {
                        Globals::ImGuiShown = true;
                        ShowWindow(hWnd, SW_SHOW);
                    }
                }
            }
            return 0;
        }
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}