#include <wndproc.h>

#include <imgui_lib.h>
#include <globals.h>
#include <tasks.h>
#include <bind.h>

#include "imgui.h"

#include <iostream>
#include <tchar.h>
#include <thread>

#include <windows.h>
#include <iostream>

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            Binds::KeyPressed.fire(p->vkCode);
        }

        if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            Binds::KeyReleased.fire(p->vkCode);
        }
    }

    return CallNextHookEx(Globals::g_hHook, nCode, wParam, lParam);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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