#pragma once

#include <imgui.h>

#include <general.hpp>
#include <globals.hpp>
#include <bind.hpp>

#include <type_traits>
#include <concepts>

struct Elements {
    static inline std::atomic<UINT> NextId = 1;
};

template <typename T>
concept BoolIntFloat = 
    std::same_as<T, bool> || 
    std::same_as<T, int>  || 
    std::same_as<T, float>;

template <typename T>
concept IntFloat =
    std::same_as<T, int> ||
    std::same_as<T, float>;

template <BoolIntFloat T>
struct Element {
    const std::string Label;
    std::atomic<T>* Flag;
    UINT Id;

    Element(const std::string& Label)
    : Label(Label + "##" + std::to_string(Elements::NextId.load())),
      Id(Elements::NextId.load()) {
        UINT NextIdValue = Elements::NextId.load();
        NextIdValue += 1;
        Elements::NextId.store(NextIdValue);
        
        if constexpr (std::is_same_v<T, bool>) {
            Flag = &Globals::BooleanFlags[Label];
        } else if constexpr (std::is_same_v<T, int>) {
            Flag = &Globals::IntSliderFlags[Label];
        } else if constexpr (std::is_same_v<T, float>) {
            Flag = &Globals::FloatSliderFlags[Label];
        } else {
            static_assert(sizeof(T) == 0, "Unsupported type");
        }
    }

    virtual void Update() = 0;

    virtual ~Element() noexcept = default;
};

struct Checkbox : Element<bool> {
    std::unique_ptr<BlankImGuiBind> Bind;

    Checkbox(
        const std::string& Label,
        const bool DefaultValue = false,
        std::unique_ptr<BlankImGuiBind> Bind = nullptr
    ) : Element<bool>(Label),
        Bind(std::move(Bind)) {
        nlohmann::json& JsonBooleanFlags = Globals::JsonConfig["BooleanFlags"];
        Globals::BooleanFlags[Label].store(JsonIndexDefault(JsonBooleanFlags, Label, DefaultValue));
    }

    void Update() override {
        bool Toggle = Flag->load();
        if (ImGui::Checkbox(Label.c_str(), &Toggle)) Flag->store(Toggle);
        if (Bind) Bind->Update();
    }
};

template <IntFloat T>
struct Slider : Element<T> {
    std::unique_ptr<BlankImGuiBind> Bind;
    const T Min, Max;
    const std::string Format;
    const char* LabelCStr;
    const char* FormatCStr;

    Slider(
        const std::string& Label,
        const T DefaultValue = 0,
        const T Min = 0,
        const T Max = 100,
        const std::string Format = "",
        std::unique_ptr<BlankImGuiBind> Bind = nullptr
    ) : Element<T>(Label),
        Min(Min),
        Max(Max),
        Format(Format.empty() ? (std::is_same_v<T, int> ? "%d" : "%.2f") : Format),
        Bind(std::move(Bind)) {
        LabelCStr = this->Label.c_str();
        FormatCStr = this->Format.c_str();
        nlohmann::json& JsonSliderFlags = Globals::JsonConfig[std::is_same_v<T, int> ? "IntSliderFlags" : "FloatSliderFlags"];
        if constexpr (std::is_same_v<T, int>) 
            Globals::IntSliderFlags[Label].store(JsonIndexDefault(JsonSliderFlags, Label, DefaultValue));
        else if constexpr(std::is_same_v<T, float>) 
            Globals::FloatSliderFlags[Label].store(JsonIndexDefault(JsonSliderFlags, Label, DefaultValue));
    }

    void Update() override {
        T Value = this->Flag->load();
        bool Changed;
        
        if constexpr (std::is_same_v<T, int>)
            Changed = ImGui::SliderInt(LabelCStr, &Value, Min, Max, FormatCStr);
        else if constexpr (std::is_same_v<T, float>)
            Changed = ImGui::SliderFloat(LabelCStr, &Value, Min, Max, FormatCStr);

        if (Changed) {
            Value = std::clamp(Value, Min, Max);
            this->Flag->store(Value);
        }
        
        if (Bind) Bind->Update();
    }
};