#pragma once

#include <windows.h>
#include <gdiplus.h>

inline HICON LoadPngAsIcon(const wchar_t* file) {
    Gdiplus::Bitmap bitmap(file);
    HICON hIcon = nullptr;
    bitmap.GetHICON(&hIcon);
    return hIcon;
}