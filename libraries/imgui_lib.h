#pragma once

#include <algorithm>
#include <windows.h>
#include <iostream>
#include <string>
#include <array>

#include "imgui.h"

bool CreateDeviceD3D(HWND hWnd);
void CreateRenderTarget();
void CleanupRenderTarget();
void CleanupDeviceD3D();
void SetImGuiScale(float scale);

template<size_t N>
constexpr std::array<int, 2> GetTextElementsInfo(const std::array<int, N>& arr) {
    int sum = 0;
    for (size_t i = 0; i < N; ++i) {
        sum += arr[i];
    }
    return {sum, static_cast<int>(N)};
}

inline bool SetNextWindowSize(
    const ImGuiStyle& Style = ImGui::GetStyle(),
    int FixedWidth = 100,
    int VerticalElements = 0,
    bool NoTitleBarHeight = false,
    std::array<int, 2> TextElementsInfo = {}
) {
    const float FrameHeight = ImGui::GetFrameHeight();
    const float TitleBarHeight = NoTitleBarHeight ? 0 : ImGui::GetFontSize() + Style.FramePadding.y * 2.0f;
    const float ItemSpacing = Style.ItemSpacing.y;
    const float TextLineHeight = ImGui::GetTextLineHeight();
    const float WindowPadding = Style.WindowPadding.y;

    const int LineWraps = TextElementsInfo[0];
    const int TextElements = TextElementsInfo[1];

    int MinusOne = VerticalElements - 1;
    if (MinusOne < 0) MinusOne = 0;

    ImGui::SetNextWindowSize(ImVec2(
        FixedWidth,
        WindowPadding * 2
      + ItemSpacing * (VerticalElements - 1)
      + TitleBarHeight
      + FrameHeight * VerticalElements
      + TextElements * ItemSpacing
      + LineWraps * TextLineHeight
    ), ImGuiCond_Always);

    return true;
};

inline float SetToRightOfElement(const ImGuiStyle& Style, const char* Text = "", const int SizeOffset = 0) {
    const ImVec2 LastElementSize = ImGui::GetItemRectSize();
    const float LastElementWidth = LastElementSize.x;
    const float Indentation = LastElementWidth + Style.ItemInnerSpacing.y;
    
    const float WindowWidth = ImGui::GetWindowWidth();
    const float WindowPadding = Style.WindowPadding.x;

    const float TextWidth = Text[0] != '\0' ? ImGui::CalcTextSize(Text).x + WindowPadding - 3 : 0;

    ImGui::SameLine();
    ImGui::Indent(Indentation);
    ImGui::SetNextItemWidth(WindowWidth - Indentation - 2 * WindowPadding - TextWidth + SizeOffset);
    return Indentation;
};