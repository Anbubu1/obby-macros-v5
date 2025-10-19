#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <array>

#include <imgui.h>

bool CreateDeviceD3D(HWND hWnd);
void CreateRenderTarget();
void CleanupRenderTarget();
void CleanupDeviceD3D();
void SetImGuiScale(float scale);

template<size_t N>
consteval std::array<int, 2> GetTextElementsInfo(const std::array<int, N>& Array) {
    int Sum = 0;
    for (size_t i = 0; i < N; ++i)
        Sum += Array[i];
    return {Sum, static_cast<int>(N)};
}

inline ImVec2 GetNextWindowSize(
    const ImGuiStyle& Style = ImGui::GetStyle(),
    const int FixedWidth = 250,
    const int VerticalElements = 0,
    const bool NoTitleBarHeight = false,
    const std::array<int, 2> TextElementsInfo = {0, 0},
    const int Separators = 0
) {
    static const float TextLineHeight = ImGui::GetTextLineHeight();
    static const float FrameHeight = ImGui::GetFrameHeight();

    static const float WindowPadding = Style.WindowPadding.y;
    static const float ItemSpacing = Style.ItemSpacing.y;

    const float TitleBarHeight = (ImGui::GetFontSize() + Style.FramePadding.y * 2.0f) * !NoTitleBarHeight;

    const int LineWraps = TextElementsInfo[0];
    const int TextElements = TextElementsInfo[1];

    int MinusOne = (VerticalElements - 1) & -(VerticalElements > 0);

    return ImVec2(
        FixedWidth,
        WindowPadding * 2
      + ItemSpacing * (VerticalElements - 1)
      + TitleBarHeight
      + FrameHeight * VerticalElements
      + (TextElements + Separators) * ItemSpacing
      + LineWraps * TextLineHeight
    );
}

inline bool SetNextWindowSize(const ImVec2 WindowSize) {
    ImGui::SetNextWindowSize(WindowSize);
    return true;
}

inline bool ComboFromStringVector(const char* Label, std::string* CurrentItem, const std::vector<std::string>* Items) {
    if (!CurrentItem || !Items) [[unlikely]] return false;

    int CurrentIndex = 0;
    for (int i = 0; i < (int)Items->size(); ++i) {
        if ((*Items)[i] == *CurrentItem) [[likely]] {
            CurrentIndex = i;
            break;
        }
    }

    bool Changed = ImGui::Combo(Label, &CurrentIndex, [](void* Data, const int Index, const char** Output) {
        auto& Vector = *static_cast<const std::vector<std::string>*>(Data);

        if (Index < 0 || Index >= (int)Vector.size()) [[unlikely]]
            return false;

        *Output = Vector[Index].c_str();
        return true;
    }, (void*)Items, (int)Items->size());

    if (Changed && CurrentIndex >= 0 && CurrentIndex < (int)Items->size())
        *CurrentItem = (*Items)[CurrentIndex];

    return Changed;
}