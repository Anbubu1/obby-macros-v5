#pragma once

#include <signals.h>
#include <d3d11.h>

#include <unordered_map>
#include <windows.h>
#include <chrono>
#include <string>

namespace Globals {
    constexpr inline float ROBLOX_SENS_MULT = 2.75;
    
    namespace MultiSliderCallbackImGuiBindSettings {
        constexpr inline int MAX_CALLBACK_BINDS = 10;
        constexpr inline int WINDOW_WIDTH = 175;
    }

    inline bool ImGuiShown = true;
    inline std::unordered_map<std::string, bool> BooleanFlags = {};
    inline std::unordered_map<std::string, int> IntSliderFlags = {};
    inline std::unordered_map<std::string, float> FloatSliderFlags = {};

    inline Signal<const std::chrono::duration<float>> RenderStepped;

    namespace DirectX {
        inline ID3D11Device*           g_pd3dDevice = nullptr;
        inline ID3D11DeviceContext*    g_pd3dDeviceContext = nullptr;
        inline IDXGISwapChain*         g_pSwapChain = nullptr;
        inline ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
    }

    inline NOTIFYICONDATA NotifyIconData = {};
    inline HHOOK g_hHook = NULL;
    inline HWND MainWindow;
}