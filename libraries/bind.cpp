#include <bind.h>

const char* DisplayMap[256] = { nullptr };
std::unordered_map<UINT, BlankImGuiBind*> IdToBind = {};

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

bool SetRightMostButton(BlankImGuiBind* const Bind, const bool NoDummy) {
    if (!NoDummy) {
        const float RegionWidth = ImGui::GetContentRegionAvail().x;
        const float LastElementWidth = ImGui::GetItemRectSize().x;
        const float TextWidth = ImGui::CalcTextSize(Bind->Display.c_str()).x;

        const ImGuiStyle Style = ImGui::GetStyle();
        const float LeftTextMargin = Style.FramePadding.x;
        const float PaddingWidth = Style.WindowPadding.x;
        const float SpacingWidth = Style.ItemSpacing.x;

        ImGui::SameLine(); ImGui::Dummy(
            ImVec2(RegionWidth
                - TextWidth
                - 2 * LeftTextMargin
                - 2 * SpacingWidth
                - LastElementWidth, 0)
        );
    }

    ImGui::SameLine();
    const std::string Label = Bind->Display + "##" + static_cast<char>(Bind->Id);
    const bool Clicked = ImGui::Button(Label.c_str());

    return Clicked;
};

bool UpdateBind(BlankImGuiBind* const Bind, const bool NoDummy) {
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
            const UINT BoundId = *Binds::Binding;
            BlankImGuiBind* const BindingBind = IdToBind[*Binds::Binding];

            BindingBind->Display = BindingBind->Key;
            Binds::Binding = nullptr;
        } else {
            Binds::Binding = &Bind->Id;
            Bind->Display = "...";
        }
    }

    return Clicked;
}

void SetBindKey(BlankImGuiBind* const Bind, const UINT VK) {
    std::string ReadableKeyName = GetReadableKeyName(VK);

    const size_t len = strlen(ReadableKeyName.c_str());

    if (len == 1) {
        const unsigned char ASCII = static_cast<unsigned char>(ReadableKeyName[0]);

        const char* FixedKeyName = DisplayMap[ASCII];

        if (FixedKeyName == nullptr) {
            Bind->Display = ToUpper(ReadableKeyName);
            Bind->Key = ReadableKeyName;
        } else {
            Bind->Display = ToUpper(FixedKeyName);
            Bind->Key = FixedKeyName;
        }
    } else {
        Bind->Display = ToUpper(ReadableKeyName);
        Bind->Key = ReadableKeyName;
    }

    Bind->VK = VK;
};