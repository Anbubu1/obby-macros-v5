#pragma once

#include <imgui.h>

#include <general.hpp>
#include <globals.hpp>
#include <bind.hpp>

#include <type_traits>
#include <concepts>

namespace Elements {
    inline std::atomic<UINT> NextId = 1;
}

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
        if constexpr (std::is_same_v<T, int>) {
            nlohmann::json& JsonIntSliderFlags = Globals::JsonConfig["IntSliderFlags"];
            Globals::IntSliderFlags[Label].store(JsonIndexDefault(JsonIntSliderFlags, Label, DefaultValue));
        } else if constexpr(std::is_same_v<T, float>) {
            nlohmann::json& JsonFloatSliderFlags = Globals::JsonConfig["FloatSliderFlags"];
            Globals::FloatSliderFlags[Label].store(JsonIndexDefault(JsonFloatSliderFlags, Label, DefaultValue));
        }
    }

    void Update() override {
        auto Value = this->Flag->load();
        if constexpr (std::is_same_v<T, int>) {
            if (ImGui::SliderInt(this->Label.c_str(), &Value, Min, Max, this->Format.c_str())) {
                Value = std::clamp(Value, Min, Max);
                this->Flag->store(Value);
            }
        } else if constexpr (std::is_same_v<T, float>) {
            if (ImGui::SliderFloat(this->Label.c_str(), &Value, Min, Max, this->Format.c_str())) {
                Value = std::clamp(Value, Min, Max);
                this->Flag->store(Value);
            }
        }
        if (Bind) Bind->Update();
    }
};