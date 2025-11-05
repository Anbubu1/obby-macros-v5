#pragma once

#include <windows.h>
#include <wincodec.h>
#include <gdiplus.h>

HRESULT ConvertWICBitmapSourceToHBITMAP(IWICBitmapSource* pSource, HBITMAP* phBitmapOut);
HICON LoadWebpAsIcon(const wchar_t* file);