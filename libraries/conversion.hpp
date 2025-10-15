#pragma once

#include <windows.h>
#include <wincodec.h>
#include <gdiplus.h>

inline HRESULT ConvertWICBitmapSourceToHBITMAP(IWICBitmapSource* pSource, HBITMAP* phBitmapOut) {
    if (!pSource || !phBitmapOut) return E_INVALIDARG;
    *phBitmapOut = nullptr;

    IWICBitmapSource* pConverted = nullptr;
    HRESULT hResult = WICConvertBitmapSource(GUID_WICPixelFormat32bppBGRA, pSource, &pConverted);
    if (FAILED(hResult)) {
        hResult = WICConvertBitmapSource(GUID_WICPixelFormat32bppBGR, pSource, &pConverted);
        if (FAILED(hResult)) return hResult;
    }

    UINT Width = 0, Height = 0;
    hResult = pConverted->GetSize(&Width, &Height);
    if (FAILED(hResult)) {
        pConverted->Release();
        return hResult;
    }
    if (Width == 0 || Height == 0) {
        pConverted->Release();
        return E_FAIL;
    }

    BITMAPINFO BitmapInfo = {};
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = (LONG)Width;
    BitmapInfo.bmiHeader.biHeight = -(LONG)Height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    void* pvBits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(nullptr, &BitmapInfo, DIB_RGB_COLORS, &pvBits, nullptr, 0);
    if (!hBitmap || !pvBits) {
        if (pConverted) pConverted->Release();
        return HRESULT_FROM_WIN32(GetLastError());
    }

    UINT cbStride = Width * 4;
    UINT cbBuffer = cbStride * Height;
    hResult = pConverted->CopyPixels(nullptr, cbStride, cbBuffer, static_cast<BYTE*>(pvBits));
    pConverted->Release();

    if (FAILED(hResult)) {
        DeleteObject(hBitmap);
        return hResult;
    }

    *phBitmapOut = hBitmap;
    return S_OK;
}

inline HICON LoadWebpAsIcon(const wchar_t* file) {
    IWICBitmapFrameDecode* pFrame = nullptr;
    IWICImagingFactory* pFactory = nullptr;
    IWICBitmapDecoder* pDecoder = nullptr;
    HICON hIcon = nullptr;
    HBITMAP hBitmap = nullptr;
    Gdiplus::Bitmap* pGdiBitmap = nullptr;

    HRESULT hResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    bool coInit = SUCCEEDED(hResult);

    hResult = CoCreateInstance(
        CLSID_WICImagingFactory, nullptr,
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory)
    );
    if (FAILED(hResult)) goto cleanup;

    hResult = pFactory->CreateDecoderFromFilename(
        file, nullptr, GENERIC_READ,
        WICDecodeMetadataCacheOnLoad, &pDecoder
    );
    if (FAILED(hResult)) goto cleanup;

    hResult = pDecoder->GetFrame(0, &pFrame);
    if (FAILED(hResult)) goto cleanup;

    hResult = ConvertWICBitmapSourceToHBITMAP(pFrame, &hBitmap);
    if (FAILED(hResult) || hBitmap == nullptr) goto cleanup;

    pGdiBitmap = Gdiplus::Bitmap::FromHBITMAP(hBitmap, nullptr);
    if (pGdiBitmap) {
        Gdiplus::Status st = pGdiBitmap->GetHICON(&hIcon);
        delete pGdiBitmap;
        pGdiBitmap = nullptr;
    }

cleanup:
    if (pFrame) pFrame->Release();
    if (pDecoder) pDecoder->Release();
    if (pFactory) pFactory->Release();
    if (coInit) CoUninitialize();
    if (hBitmap) DeleteObject(hBitmap);

    return hIcon;
}