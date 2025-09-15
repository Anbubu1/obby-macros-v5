#include <init.h>

#include <wndproc.h>

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