#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <array>

#include <imgui.h>

struct WindowSizeParams {
    int WindowWidth = 250;
    int NonTextElements = 0;
    std::array<int, 2> TextElementsInfo{0, 0};
    int Separators = 0;
};

bool CreateDeviceD3D(HWND hWnd);
void CreateRenderTarget();
void CleanupRenderTarget();
void CleanupDeviceD3D();
void SetImGuiScale(float scale);

template<size_t N>
consteval std::array<int, 2> ComputeTextLayoutInfo(const std::array<int, N>& Array) {
    int Sum = 0;
    for (size_t i = 0; i < N; ++i)
        Sum += Array[i];
    return {Sum, static_cast<int>(N)};
}

ImVec2 GetNextWindowSize(
    const ImGuiStyle& Style = ImGui::GetStyle(),
    const int FixedWidth = 250,
    const int VerticalElements = 0,
    const bool IgnoreTitleBar = false,
    const std::array<int, 2> TextElementsInfo = {0, 0},
    const int Separators = 0
);

ImVec2 GetNextWindowSize(
    const ImGuiStyle& Style = ImGui::GetStyle(),
    const WindowSizeParams Parameters = WindowSizeParams{},
    const int WindowFlags = 0
);

inline bool SetNextWindowSize(const ImVec2 WindowSize) {
    ImGui::SetNextWindowSize(WindowSize);
    return true;
}

bool ComboFromStringVector(
    const char* Label,
    std::string& CurrentItem,
    const std::vector<std::string>& Items
);