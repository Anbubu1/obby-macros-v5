#include <init.hpp>

#include <wndproc.hpp>

#include "imgui.h"
#include <windows.h>
#include <tchar.h>

WNDCLASSEX InitialiseWindow(HINSTANCE hInstance) {
    WNDCLASSEX wc;
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_CLASSDC;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = _T("ImGui Example");
    wc.hIconSm       = NULL;

    return wc;
}

struct ACCENT_POLICY {
    int nAccentState;
    int nFlags;
    int nColor;
    int nAnimationId;
};

struct WINDOWCOMPOSITIONATTRIBDATA {
    int nAttribute;
    PVOID pData;
    SIZE_T cbData;
};

enum AccentState {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_ENABLE_HOSTBACKDROP = 5,
    ACCENT_INVALID_STATE = 6
};

typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

void EnableBlur(HWND hwnd) {
    HMODULE hUser = GetModuleHandle("user32.dll");
    auto SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hUser, "SetWindowCompositionAttribute");

    if (SetWindowCompositionAttribute) {
        ACCENT_POLICY policy = {};
        policy.nAccentState = ACCENT_ENABLE_TRANSPARENTGRADIENT;
        policy.nFlags = 2;
        policy.nColor = 0x99000000;

        WINDOWCOMPOSITIONATTRIBDATA data = {};
        data.nAttribute = 19;
        data.pData = &policy;
        data.cbData = sizeof(policy);

        SetWindowCompositionAttribute(hwnd, &data);
    }
}