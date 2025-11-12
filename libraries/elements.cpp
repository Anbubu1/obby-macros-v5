#include <elements.hpp>
#include <fmt/args.h>

Window::Window(
    const std::string& Label,
    const int WindowFlags,
    const WindowSizeParams Parameters,
    std::vector<std::unique_ptr<Element>> GivenElements
) : Label(Label),
    WindowFlags(WindowFlags),
    WindowSize(GetNextWindowSize(ImGui::GetStyle(), Parameters)),
    Elements(std::move(GivenElements)) {}

void Window::Update() const {
    ImGui::SetNextWindowSize(this->WindowSize);
    ImGui::Begin(this->Label.c_str(), nullptr, WindowFlags);
    for (auto& Element : this->Elements)
        Element->Update();
    ImGui::End();
}


Element::Element() : Id(Elements::NextId.load()) {}

PushStyle::PushStyle(const ElementStyle Style) : Style(Style) {};
PopStyle::PopStyle(const ElementStyle Style) : Style(Style) {};

void PushStyle::Update() {
    switch (this->Style) {
        case ElementStyle::TextWrapping: ImGui::PushTextWrapPos();
    }
}

void PopStyle::Update() {
    switch (this->Style) {
        case ElementStyle::TextWrapping: ImGui::PopTextWrapPos();
    }
}


void Separator::Update() {
    ImGui::Separator();
}


std::string TooltipHandler::Get() const {
    if (this->IsLiteral) return this->Format;

    fmt::dynamic_format_arg_store<fmt::format_context> Store;
    for (auto& Arg : FormatArguments)
        Store.push_back(*Arg);

    return fmt::vformat(Format, Store);
}


LabelledElement::LabelledElement(
    const LabelledElementParams Parameters
) : Label(Parameters.Label), Tooltip(Parameters.Tooltip) {}

void LabelledElement::SetTooltip() const {
    if (this->Tooltip.has_value())
        ImGui::SetItemTooltip("%s", this->Tooltip.value().Get().c_str());
}


Button::Button(
    const ButtonParams Parameters
) : LabelledElement({
    .Label = Parameters.Label,
    .Tooltip = Parameters.Tooltip
}), Callback(Parameters.Callback) {};

void Button::Update() {
    if (ImGui::Button(this->Label.c_str()))
        this->Callback();
    this->SetTooltip();
};


Text::Text(
    const TextParams Parameters
) : LabelledElement({
    .Label = Parameters.Label,
    .Tooltip = Parameters.Tooltip
}), Color(Parameters.Color) {}

void Text::Update() {
    if (this->Color.has_value())
        ImGui::TextColored(this->Color.value(), "%s", this->Label.c_str());
    else
        ImGui::Text("%s", this->Label.c_str());
    this->SetTooltip();
}


template <BoolIntFloat type>
InteractiveFlagElement<type>::InteractiveFlagElement(
    const InteractiveFlagElementParams<type> Parameters
) : LabelledElement({
    .Label = Parameters.Label,
    .Tooltip = Parameters.Tooltip
}) {
    Elements::NextId.store(Id + 1);

    if constexpr (std::is_same_v<type, bool>)
        Flag = &Globals::BooleanFlags[Label];
    else if constexpr (std::is_same_v<type, int>)
        Flag = &Globals::IntSliderFlags[Label];
    else if constexpr (std::is_same_v<type, float>)
        Flag = &Globals::FloatSliderFlags[Label];
    else
        static_assert(sizeof(type) == 0, "Unsupported type");
}


Checkbox::Checkbox(const CheckboxParams Parameters)
  : InteractiveFlagElement<bool>({
        .Label = Parameters.Label,
        .Tooltip = Parameters.Tooltip,
        .DefaultValue = Parameters.DefaultValue
    }),
    Bind(std::move(Parameters.Bind)) {
    nlohmann::json& JsonBooleanFlags = Globals::JsonConfig["BooleanFlags"];
    Globals::BooleanFlags[Label].store(JsonIndexDefault(JsonBooleanFlags, Label, Parameters.DefaultValue));
}

void Checkbox::Update() {
    bool Toggle = Flag->load();
    if (ImGui::Checkbox(Label.c_str(), &Toggle)) Flag->store(Toggle);
    this->SetTooltip();
    if (this->Bind.has_value()) this->Bind.value()->Update();
}


template <IntFloat type>
Slider<type>::Slider(
    const SliderParams<type> Parameters
) : InteractiveFlagElement<type>({
        .Label = Parameters.Label,
        .Tooltip = Parameters.Tooltip,
        .DefaultValue = Parameters.DefaultValue
    }),
    Bind(std::move(Parameters.Bind)),
    Min(Parameters.Min),
    Max(Parameters.Max),
    Format(Parameters.Format.empty() ? (std::is_same_v<type, int> ? "%d" : "%.2f") : Parameters.Format) {
    nlohmann::json& JsonSliderFlags = Globals::JsonConfig[std::is_same_v<type, int> ? "IntSliderFlags" : "FloatSliderFlags"];
    if constexpr (std::is_same_v<type, int>){
        Globals::IntSliderFlags[
            Parameters.Label
        ].store(JsonIndexDefault(
            JsonSliderFlags,
            Parameters.Label,
            Parameters.DefaultValue
        ));}
    else if constexpr(std::is_same_v<type, float>) 
        Globals::FloatSliderFlags[
            Parameters.Label
        ].store(JsonIndexDefault(
            JsonSliderFlags,
            Parameters.Label,
            Parameters.DefaultValue
        ));
}

template <IntFloat type>
void Slider<type>::Update() {
    if (this->ItemWidth == -1) [[unlikely]]
        this->ItemWidth = ImGui::GetContentRegionAvail().x
                        - ImGui::GetStyle().ItemInnerSpacing.x
                        - ImGui::CalcTextSize(this->Label.c_str()).x;

    ImGui::SetNextItemWidth(this->ItemWidth);

    type Value = this->Flag->load();

    bool Changed;
    if constexpr (std::is_same_v<type, int>)
        Changed = ImGui::SliderInt(
            this->Label.c_str(),
            &Value, Min, Max, this->Format.c_str()
        );
    else if constexpr (std::is_same_v<type, float>)
        Changed = ImGui::SliderFloat(
            this->Label.c_str(),
            &Value, Min, Max, this->Format.c_str()
        );

    this->SetTooltip();

    if (Changed) [[unlikely]]
        this->Flag->store(std::clamp(Value, Min, Max));
    
    if (this->Bind.has_value()) this->Bind.value()->Update();
}

template struct InteractiveFlagElementParams<int>;
template struct InteractiveFlagElementParams<float>;

template struct SliderParams<int>;
template struct SliderParams<float>;

template struct Slider<int>;
template struct Slider<float>;