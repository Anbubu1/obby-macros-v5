#include <bind.h>

const char* DisplayMap[256] = { nullptr };
std::unordered_map<UINT, BlankImGuiBind*> IdToBind = {};
UINT NextId = 0;

struct DisplayMapInitializer {
    DisplayMapInitializer() {
        DisplayMap['0'] = "ZERO";
        DisplayMap['1'] = "ONE";
        DisplayMap['2'] = "TWO";
        DisplayMap['3'] = "THREE";
        DisplayMap['4'] = "FOUR";
        DisplayMap['5'] = "FIVE";
        DisplayMap['6'] = "SIX";
        DisplayMap['7'] = "SEVEN";
        DisplayMap['8'] = "EIGHT";
        DisplayMap['9'] = "NINE";

        DisplayMap['`'] = "GRAVE";
        DisplayMap['/'] = "SLASH";
        DisplayMap['\\'] = "BSLASH";
        DisplayMap['-'] = "MINUS";
        DisplayMap['='] = "EQUAL";
        DisplayMap['\''] = "QUOTE";
        DisplayMap[';'] = "SCOLON";
        DisplayMap[','] = "COMMA";
        DisplayMap['.'] = "PERIOD";
        DisplayMap['['] = "LSQBRKT";
        DisplayMap[']'] = "RSQBRKT";
    }
};

static DisplayMapInitializer _displayMapInit;

bool SetRightMostButton(BlankImGuiBind* Bind, bool NoDummy) {
    if (!NoDummy) {
        const ImVec2 RegionSize = ImGui::GetContentRegionAvail();
        const float RegionWidth = RegionSize.x;

        const ImVec2 LastElementSize = ImGui::GetItemRectSize();
        const float LastElementWidth = LastElementSize.x;

        const ImGuiStyle Style = ImGui::GetStyle();
        const float LeftTextMargin = Style.FramePadding.x;
        const float PaddingWidth = Style.WindowPadding.x;
        const float SpacingWidth = Style.ItemSpacing.x;

        const ImVec2 TextSize = ImGui::CalcTextSize(Bind->Display);
        const float TextWidth = TextSize.x;

        ImGui::SameLine(); ImGui::Dummy(
            ImVec2(RegionWidth
                - TextWidth
                - 2 * LeftTextMargin
                - 2 * SpacingWidth
                - LastElementWidth, 0)
        );
    }

    ImGui::SameLine();
    std::string Label = std::string(Bind->Display) + "##" + static_cast<char>(Bind->Id);
    const bool Clicked = ImGui::Button(Label.c_str());

    return Clicked;
};

bool UpdateBind(BlankImGuiBind* Bind, bool NoDummy) {
    const bool Clicked = SetRightMostButton(Bind, NoDummy);

    switch (Bind->Mode) {
        case BindMode::None:
            break;

        case BindMode::Always: {
            ImGui::SetItemTooltip("The checkbox is always true.");
            break;
        }

        case BindMode::Hold: {
            ImGui::SetItemTooltip("Hold the bind to toggle the checkbox!");
            break;
        }

        case BindMode::Toggle: {
            ImGui::SetItemTooltip("Press the bind to toggle the checkbox!");
            break;
        }
    }

    if (Clicked) {
        if (Binds::Binding != nullptr) {
            UINT BoundId = *Binds::Binding;
            BlankImGuiBind* BindingBind = IdToBind[BoundId];

            BindingBind->Display = BindingBind->Key;
            Binds::Binding = nullptr;
        } else {
            Binds::Binding = &Bind->Id;
            Bind->Display = "...";
        }
    }

    return Clicked;
}

void SetBindKey(BlankImGuiBind* Bind, UINT VK) {
    const char* ReadableKeyName = GetReadableKeyNameChar(VK);

    const size_t len = strlen(ReadableKeyName);

    if (len == 1) {
        unsigned char ASCII = static_cast<unsigned char>(ReadableKeyName[0]);

        if (DisplayMap[ASCII] == nullptr) {
            Bind->Display = to_upper(ReadableKeyName);
            Bind->Key = ReadableKeyName;
        } else {
            const char* FixedKeyName = DisplayMap[ASCII];
            Bind->Display = to_upper(FixedKeyName);
            Bind->Key = FixedKeyName;
        }
    } else {
        Bind->Display = to_upper(ReadableKeyName);
        Bind->Key = ReadableKeyName;
    }

    Bind->VK = VK;
};