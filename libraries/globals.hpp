#pragma once

#include <signals.hpp>
#include <json.hpp>

#include <d3d11.h>

#include <unordered_map>
#include <windows.h>
#include <fstream>
#include <chrono>
#include <string>

namespace Globals {
    constexpr inline float ROBLOX_SENS_MULT = 2.75;
    constexpr inline int OPEN_CLOSE_KEY = VK_INSERT;

    inline nlohmann::json GetDefaultConfig() {
        return {
            {"FloatSliderFlags", {}},
            {"IntSliderFlags", {}},
            {"BooleanFlags", {}}
        };
    }

    inline nlohmann::json JSONConfig = Globals::GetDefaultConfig();

    inline std::filesystem::path ConfigPath;
    
    namespace MultiSliderCallbackImGuiBindSettings {
        constexpr inline int MAX_CALLBACK_BINDS = 5;
        constexpr inline int WINDOW_WIDTH = 175;
    }

    inline bool ImGuiShown = true;
    inline std::unordered_map<std::string, std::atomic<float>> FloatSliderFlags = {};
    inline std::unordered_map<std::string, std::atomic<int>> IntSliderFlags = {};
    inline std::unordered_map<std::string, std::atomic<bool>> BooleanFlags = {};

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