#pragma once

#include <windows_lib.hpp>
#include <signals.hpp>
#include <json.hpp>

#include <d3d11.h>

#include <unordered_map>
#include <filesystem>
#include <windows.h>
#include <chrono>
#include <string>
#include <vector>

namespace Globals {
    constexpr inline float ROBLOX_SENS_MULT = 2.75;
    constexpr inline int OPEN_CLOSE_KEY = VK_INSERT;
    constexpr inline const char* MAIN_FOLDER_NAME = "Obby-Macros";
    constexpr inline const char* CONFIG_FOLDER_NAME = "config";
    constexpr inline const char* DEFAULT_CONFIG_NAME = "default.json";
    constexpr inline const char* MAIN_CONFIG_NAME = "main_config.json";

    inline float __DebugRobloxSensitivityMultiplier = 1;

    inline std::string CurrentConfigName = DEFAULT_CONFIG_NAME;

    inline nlohmann::json GetDefaultConfig() {
        return {
            {"FloatSliderFlags", {}},
            {"IntSliderFlags", {}},
            {"BooleanFlags", {}}
        };
    }

    inline nlohmann::json GetDefaultMainConfig() {
        return {{"ConfigUsed", DEFAULT_CONFIG_NAME}};
    }

    inline std::filesystem::path MainFolderPath = []() {
        wchar_t buffer[MAX_PATH];
        if (GetEnvironmentVariableW(L"LOCALAPPDATA", buffer, MAX_PATH) == 0)
            throw std::runtime_error("Failed to get \%localappdata\%!");

        return std::filesystem::path(buffer) / MAIN_FOLDER_NAME;
    }();

    inline const std::filesystem::path ConfigFolderPath = MainFolderPath / CONFIG_FOLDER_NAME;
    inline const std::filesystem::path MainConfigPath = MainFolderPath / MAIN_CONFIG_NAME;
    
    inline std::vector<std::string> JsonConfigPaths = ListJsonFiles(ConfigFolderPath);
    inline std::filesystem::path ConfigPath = ConfigFolderPath / CurrentConfigName;
    
    inline nlohmann::json MainJsonConfig = Globals::GetDefaultMainConfig();
    inline nlohmann::json JsonConfig = Globals::GetDefaultConfig();

    inline const std::string DefaultConfigString = JsonConfig.dump();
    
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