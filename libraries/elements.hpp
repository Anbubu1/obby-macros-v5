#pragma once

#include <imgui.h>

#include <imgui_lib.hpp>
#include <general.hpp>
#include <globals.hpp>
#include <bind.hpp>

#include <types.hpp>

#include <functional>
#include <concepts>
#include <optional>
#include <vector>

struct Elements {
    static inline atomic(uint) NextId = 1;
};

template <typename type>
concept BoolIntFloat = 
    std::same_as<type, bool> || 
    std::same_as<type, int>  || 
    std::same_as<type, float>;

template <typename type>
concept IntFloat =
    std::same_as<type, int> ||
    std::same_as<type, float>;

template <typename... Args>
concept PtrStrings = (std::same_as<Args, std::string*> && ...);

struct Element {
    const uint Id;
    explicit Element();
    virtual void Update() = 0;
    virtual ~Element() noexcept = default;
};


struct Window {
    const std::string Label;
    const int WindowFlags;
    const ImVec2 WindowSize;
    std::vector<std::unique_ptr<Element>> Elements;
    explicit Window(
        const std::string& Label,
        const int WindowFlags,
        const WindowSizeParams Parameters,
        std::vector<std::unique_ptr<Element>> GivenElements
    );
    void Update() const;
};


enum class ElementStyle {
    TextWrapping
};

struct PushStyle final : Element {
    const ElementStyle Style;
    PushStyle(const ElementStyle Style);
    void Update() override;
};

struct PopStyle final : Element {
    const ElementStyle Style;
    PopStyle(const ElementStyle Style);
    void Update() override;
};


struct Separator final : Element {
    void Update() override;
};


struct TooltipHandler {
    const bool IsLiteral = true;
    const std::string Format = "";
    std::vector<std::string*> FormatArguments;
    
    template <typename... Args>
    requires PtrStrings<Args...>
    TooltipHandler(
        const std::string& Format,
        Args&&... Arguments
    ) : IsLiteral(sizeof...(Args) == 0),
        Format(Format) {
        (FormatArguments.push_back(std::forward<Args>(Arguments)), ...);
    };

    std::string Get() const;
};


struct LabelledElementParams {
    const std::string& Label;
    const std::optional<TooltipHandler> Tooltip = std::nullopt;
};

struct LabelledElement : Element {
    const std::string Label;
    const std::optional<TooltipHandler> Tooltip;
    explicit LabelledElement(const LabelledElementParams Parameters);
    void SetTooltip() const;
};


struct ButtonParams {
    const std::string& Label;
    const std::optional<TooltipHandler> Tooltip = std::nullopt;
    std::function<void()> Callback;
};

struct Button final : LabelledElement {
    std::function<void()> Callback;
    explicit Button(const ButtonParams Parameters);
    void Update() override;
};


struct TextParams {
    const std::string& Label;
    const std::optional<TooltipHandler> Tooltip = std::nullopt;
    const std::optional<ImVec4> Color = std::nullopt;
};

struct Text final : LabelledElement {
    const std::optional<ImVec4> Color;
    explicit Text(const TextParams Parameters);
    void Update() override;
};


template <BoolIntFloat type>
struct InteractiveFlagElementParams {
    const std::string& Label;
    const std::optional<TooltipHandler> Tooltip = std::nullopt;
    const type DefaultValue;
};

template <BoolIntFloat type>
struct InteractiveFlagElement : LabelledElement {
    atomic(type)* Flag;
    explicit InteractiveFlagElement(
        const InteractiveFlagElementParams<type> Parameters
    );
};


struct CheckboxParams {
    const std::string& Label;
    const std::optional<TooltipHandler> Tooltip = std::nullopt;
    const bool DefaultValue = false;
    std::optional<std::unique_ptr<BaseImGuiBind>>&& Bind = std::nullopt;
};

struct Checkbox final : InteractiveFlagElement<bool> {
    std::optional<std::unique_ptr<BaseImGuiBind>> Bind;
    explicit Checkbox(const CheckboxParams Parameters);
    void Update() override;
};


template <IntFloat type>
struct SliderParams {
    const std::string& Label;
    const std::optional<TooltipHandler> Tooltip = std::nullopt;
    const type DefaultValue = 100;
    const type Min = 0;
    const type Max = 0;
    const std::string Format = "";
    std::optional<std::unique_ptr<BaseImGuiBind>>&& Bind = std::nullopt;
};

template <IntFloat type>
struct Slider final : InteractiveFlagElement<type> {
    std::optional<std::unique_ptr<BaseImGuiBind>> Bind;
    const type Min = 0, Max = 0;
    const std::string Format = "";
    float ItemWidth = -1;
    explicit Slider(const SliderParams<type> Parameters);
    void Update() override;
};





template<typename... Ts>
inline std::vector<std::unique_ptr<Element>> MakeElementVector(Ts&&... Arguments) {
    std::vector<std::unique_ptr<Element>> Vector;
    (Vector.push_back(std::forward<Ts>(Arguments)), ...);
    return Vector;
}