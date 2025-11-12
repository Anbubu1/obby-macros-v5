#include <bind.hpp>

std::unordered_map<uint, BaseImGuiBind*> IdToBind = {};

bool SetRightMostButton(BaseImGuiBind* const Bind, const bool NoDummy) {
    if (!NoDummy) {
        const float RegionWidth = ImGui::GetContentRegionAvail().x;
        const float LastElementWidth = ImGui::GetItemRectSize().x;
        const float TextWidth = ImGui::CalcTextSize(Bind->Display.c_str()).x;

        const ImGuiStyle Style = ImGui::GetStyle();
        const float LeftTextMargin = Style.FramePadding.x;
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
}

bool UpdateBind(BaseImGuiBind* const Bind, const bool NoDummy) {
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
            BaseImGuiBind* const BindingBind = IdToBind[*Binds::Binding];

            BindingBind->Display = BindingBind->Key;
            Binds::Binding = nullptr;
        } else {
            Binds::Binding = &Bind->Id;
            Bind->Display = "...";
        }
    }

    return Clicked;
}

void SetBindKey(BaseImGuiBind* const Bind, const uint VK) {
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
}

bool UpdateKey(BaseImGuiBind* const Bind, const uint VK) {
    if (Binds::Binding == &Bind->Id && Globals::ImGuiShown) {
        SetBindKey(Bind, VK);
        Binds::Binding = nullptr;
        return true;
    }
    
    return false;
}


BaseImGuiBind::BaseImGuiBind(const uint VK, const BindMode Mode)
  : Mode(Mode), Id(Binds::NextId) {
    IdToBind[Id] = this;
    Binds::NextId += 1;
    if (VK != UNUSABLE_VK)
        SetBindKey(this, VK);
}

void BaseImGuiBind::Update() {UpdateBind(this);}


ImGuiBind::ImGuiBind(const uint VK, const BindMode Mode)
  : BaseImGuiBind(VK, Mode) {
    this->Type = BindType::Normal;
    OnPressConnection = Binds::KeyPressed.connect([this](uint VK_Key) {
        if (UpdateKey(this, VK_Key)) return;
        if (this->Mode == BindMode::Toggle && VK_Key == this->VK)
            this->Flag = !this->Flag;
    });
}

void ImGuiBind::Update() {
    UpdateBind(this);

    switch (this->Mode) {
        case BindMode::Hold: {
            this->Flag = GetAsyncKeyState(static_cast<bool>(this->VK));
            return;
        }

        case BindMode::None: {
            this->Flag = false;
            return;
        }

        case BindMode::Always: {
            this->Flag = true;
            return;
        }

        case BindMode::Toggle: {
            return;
        }
    }
}

ImGuiBind::~ImGuiBind() noexcept {
    this->OnPressConnection.disconnect();
}

CallbackImGuiBind::CallbackImGuiBind(
    const std::function<void()> Callback,
    const BindMode BindMode,
    const uint VK
) : BaseImGuiBind(VK, BindMode), Callback(Callback) {
    Type = BindType::Callback;
    OnPressConnection = Binds::KeyPressed.connect([this](const uint VK_Key) {
        if (UpdateKey(this, VK_Key)) return;
        if (VK_Key != this->VK) return;
        if (this->Mode != BindMode::Hold) {
            this->Callback();
            return;
        }

        this->Holding = true;
        auto HoldingConnection = std::make_shared<Connection<uint>>(Connection<uint>{});
        *HoldingConnection = Binds::KeyReleased.connect([this, HoldingConnection](const uint VK_Key) {
            if (this->VK != VK_Key) return;
            HoldingConnection->disconnect();
            this->Holding = false;
        });

        std::jthread([this](std::stop_token stoken) {
            using namespace std::chrono;

            while (!stoken.stop_requested() && this->Holding) {
                const auto Start = high_resolution_clock::now();

                this->Callback();

                const auto Elapsed = high_resolution_clock::now() - Start;
                if (Elapsed < milliseconds(10)) {
                    std::this_thread::sleep_for(
                        milliseconds(10) - duration_cast<milliseconds>(Elapsed)
                    );
                }
            }
        }).detach();
    });
}

CallbackImGuiBind::~CallbackImGuiBind() noexcept {
    this->OnPressConnection.disconnect();
}


SliderCallbackImGuiBind::SliderCallbackImGuiBind(
    const std::function<void(int)> Callback,
    const std::string Label,
    const int DefaultValue,
    const int Min,
    const int Max,
    const std::string Format,
    const BindMode BindMode
)  : BaseImGuiBind(UNUSABLE_VK, BindMode),
    Label(Label + "##" + std::to_string(Binds::NextId)),
    Format(Format),
    ButtonLabel("-##" + std::to_string(Binds::NextId)),
    Callback(Callback),
    Value(DefaultValue),
    Min(Min),
    Max(Max) {
    Type = BindType::Callback;
    OnPressConnection = Binds::KeyPressed.connect([this](const uint VK_Key) {
        if (UpdateKey(this, VK_Key)) return;
        if (VK_Key != this->VK) return;
        if (this->Mode != BindMode::Hold) {
            this->Callback(Value);
            return;
        }
        this->Holding = true;
        auto HoldingConnection = std::make_shared<Connection<uint>>(Connection<uint>{});

        *HoldingConnection = Binds::KeyReleased.connect([this, HoldingConnection](const uint VK_Key) {
            if (this->VK != VK_Key) return;
            HoldingConnection->disconnect();
            this->Holding = false;
        });

        std::jthread([this](std::stop_token stoken) {
            using namespace std::chrono;
            while (!stoken.stop_requested() && this->Holding) {
                const auto Start = high_resolution_clock::now();

                this->Callback(this->Value);

                const auto Elapsed = high_resolution_clock::now() - Start;
                if (Elapsed < milliseconds(10)) {
                    std::this_thread::sleep_for(milliseconds(10) - duration_cast<milliseconds>(Elapsed));
                }
            }
        }).detach();
    });
}