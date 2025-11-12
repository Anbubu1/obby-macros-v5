#pragma once

#include <unordered_map>
#include <functional>
#include <string>

#include <general.hpp>
#include <signals.hpp>
#include <imgui.h>

#include <imgui_lib.hpp>
#include <globals.hpp>
#include <windows.h>

#include <types.hpp>

struct Binds {
    inline static uint* Binding = nullptr;
    inline static uint* OpenedMultiSliderWindow = nullptr;
    inline static Signal<uint> KeyPressed;
    inline static Signal<uint> KeyReleased;
    inline static uint NextId = 1;
};

enum class BindMode {
    Hold,   // Hold the bind for the flag to be true
    None,   // Does nothing, flag is always false
    Always, // Does nothing, flag is always true
    Toggle, // Press the bind for the flag to switch
};

enum class BindType {
    None,
    Normal,
    Callback,
    Multi
};

constexpr uint UNUSABLE_VK = 0x0FFF;

struct BaseImGuiBind;

extern inline constexpr auto DisplayMap = []{
    std::array<const char*, 256> Map{};
    Map['0'] = "ZERO";
    Map['1'] = "ONE";
    Map['2'] = "TWO";
    Map['3'] = "THREE";
    Map['4'] = "FOUR";
    Map['5'] = "FIVE";
    Map['6'] = "SIX";
    Map['7'] = "SEVEN";
    Map['8'] = "EIGHT";
    Map['9'] = "NINE";
    Map['`'] = "GRAVE";
    Map['/'] = "SLASH";
    Map['\\'] = "BSLASH";
    Map['-'] = "MINUS";
    Map['='] = "EQUAL";
    Map['\''] = "QUOTE";
    Map[';'] = "SCOLON";
    Map[','] = "COMMA";
    Map['.'] = "PERIOD";
    Map['['] = "LSQBRKT";
    Map[']'] = "RSQBRKT";
    return Map;
}();

extern std::unordered_map<uint, BaseImGuiBind*> IdToBind;

bool UpdateBind(BaseImGuiBind* const Bind, const bool NoDummy = false);
void SetBindKey(BaseImGuiBind* const Bind, const uint VK);
bool SetRightMostButton(BaseImGuiBind* const Bind, const bool NoDummy = false);
bool UpdateKey(BaseImGuiBind* const Bind, const uint VK);

struct BaseImGuiBind {
    BindMode Mode = BindMode::None;
    BindType Type = BindType::None;
    std::string Display = "NONE";
    std::string Key = "NONE";
    uint VK = UNUSABLE_VK;
    uint Id = 1;

    explicit BaseImGuiBind(
        const uint VK = UNUSABLE_VK,
        const BindMode Mode = BindMode::Toggle
    );

    virtual void Update();
    virtual ~BaseImGuiBind() noexcept = default;
};

struct ImGuiBind : BaseImGuiBind {
    Connection<uint> OnPressConnection;
    bool Flag = false;

    explicit ImGuiBind(
        const uint VK = UNUSABLE_VK,
        const BindMode Mode = BindMode::Toggle
    );

    void Update() override;

    ~ImGuiBind() noexcept override;
};

struct CallbackImGuiBind : BaseImGuiBind {
    Connection<uint> OnPressConnection;
    bool Holding;

    std::function<void()> Callback;

    explicit CallbackImGuiBind(
        const std::function<void()> Callback,
        const BindMode BindMode = BindMode::None,
        const uint VK = UNUSABLE_VK
    );

    ~CallbackImGuiBind() noexcept override;
};

struct SliderCallbackImGuiBind : BaseImGuiBind {
    Connection<uint> OnPressConnection;
    std::string Label, Format, ButtonLabel;
    float SemiFinalWidth;
    bool Holding = false;
    bool Cached = false;

    std::function<void(int)> Callback;
    int Value, Min, Max;
    bool Destroying = false;

    explicit SliderCallbackImGuiBind(
        const std::function<void(int)> Callback,
        const std::string Label = "",
        const int DefaultValue = 0,
        const int Min = 0,
        const int Max = 100,
        const std::string Format = "%d",
        const BindMode BindMode = BindMode::None
    );

    void Update() override {
        using namespace std::string_literals;
        
        Destroying = ImGui::Button(ButtonLabel.c_str());

        if (!Cached) {            
            const float RegionWidth = ImGui::GetContentRegionAvail().x;
            const float LastElementWidth = ImGui::GetItemRectSize().x;
            
            const ImGuiStyle Style = ImGui::GetStyle();
            const float InnerSpacingWidth = Style.ItemInnerSpacing.x;
            const float SpacingWidth = Style.ItemSpacing.x;

            SemiFinalWidth = RegionWidth
                           - LastElementWidth
                           - SpacingWidth * 2
                           - InnerSpacingWidth * 2;

            Cached = true;
        }

        const float DisplayWidth = ImGui::CalcTextSize(this->Display.c_str()).x;

        ImGui::SameLine();
        ImGui::SetNextItemWidth(SemiFinalWidth - DisplayWidth);
        ImGui::SliderInt(Label.c_str(), &Value, Min, Max, Format.c_str());

        UpdateBind(this, true);
    }

    ~SliderCallbackImGuiBind() noexcept override {
        OnPressConnection.disconnect();
    }
};

using namespace Globals::MultiSliderCallbackImGuiBindSettings;
struct MultiSliderCallbackImGuiBind : BaseImGuiBind {
    std::vector<SliderCallbackImGuiBind> CallbackBinds;
    const std::function<void(int)> Callback;

    bool WindowToggle = false;
    const int DefaultValue, Min, Max;
    const std::string Format;
    
    explicit MultiSliderCallbackImGuiBind(
        const std::function<void(int)> cb,
        const int DefaultValue = 0,
        const int Min = 0,
        const int Max = 100,
        const std::string Format = "%d",
        const BindMode BindMode = BindMode::None
    ) : BaseImGuiBind(UNUSABLE_VK, BindMode),
        Callback(cb),
        DefaultValue(DefaultValue),
        Min(Min),
        Max(Max),
        Format(Format) {
        Type = BindType::Multi;
        Display = "BINDS";
        Key = "BINDS";
        CallbackBinds.reserve(MAX_CALLBACK_BINDS);
    }

    void Update() override {
        const bool Clicked = SetRightMostButton(this);
        ImGui::SetItemTooltip("Click to open a menu of binds for this toggle.");
        if (Clicked) WindowToggle = !WindowToggle;
        if (!WindowToggle) return;

        if (Binds::OpenedMultiSliderWindow && *Binds::OpenedMultiSliderWindow != this->Id) {
            const auto ImGuiBind = IdToBind[*Binds::OpenedMultiSliderWindow];
            if (auto* const OtherMultiBind = dynamic_cast<MultiSliderCallbackImGuiBind*>(ImGuiBind)) {
                OtherMultiBind->WindowToggle = false;
                Binds::OpenedMultiSliderWindow = nullptr;
            }
        }

        Binds::OpenedMultiSliderWindow = &this->Id;

        std::string Window = "## MultiSliderCallbackImGuiBind " + std::to_string(this->Id);

        ImGui::SetNextWindowPos(ImGui::GetItemRectMax());

        if (SetNextWindowSize(GetNextWindowSize(
            ImGui::GetStyle(),
            WINDOW_WIDTH,
            std::clamp(static_cast<int>(CallbackBinds.size()) + 1, 1, MAX_CALLBACK_BINDS),
            true
        )) && ImGui::Begin(
            Window.c_str(),
            nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize
        )) {
            for (auto Bind = CallbackBinds.begin(); Bind != CallbackBinds.end(); ) {
                Bind->Update();

                if (Bind->Destroying) {
                    Bind = CallbackBinds.erase(Bind);
                } else {
                    ++Bind;
                }
            }

            if (CallbackBinds.size() < CallbackBinds.capacity() && ImGui::Button("+")) {
                CallbackBinds.emplace_back(Callback, "", DefaultValue, Min, Max, Format, Mode);
            }

            ImGui::End();
        }
    }
};