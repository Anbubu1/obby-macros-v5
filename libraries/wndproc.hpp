#pragma once

#include <windows.h>
#include <d3d11.h>

LRESULT CALLBACK KeyboardProc(const int nCode, const WPARAM wParam, const LPARAM lParam);
LRESULT WINAPI WndProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam);