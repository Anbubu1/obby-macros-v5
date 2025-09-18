#pragma once

#include <unordered_map>
#include <functional>
#include <iostream>
#include <chrono>
#include <string>
#include <thread>
#include <mutex>

#include "signals.h"
#include "general.h"
#include "imgui.h"

#include <imgui_lib.h>
#include <globals.h>
#include <windows.h>

namespace Binds {
    inline UINT* Binding = nullptr;
    inline UINT* OpenedMultiSliderWindow = nullptr;
    inline Signal<UINT> KeyPressed;
    inline Signal<UINT> KeyReleased;
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

constexpr UINT UNUSABLE_VK = 0x0FFF;

class BlankImGuiBind;

extern const char* DisplayMap[256];
extern std::unordered_map<UINT, BlankImGuiBind*> IdToBind;
extern UINT NextId;

bool UpdateBind(BlankImGuiBind* const Bind, const bool NoDummy = false);
void SetBindKey(BlankImGuiBind* const Bind, const UINT VK);
bool SetRightMostButton(BlankImGuiBind* const Bind, const bool NoDummy = false);

class BlankImGuiBind {
public:
    BindMode Mode = BindMode::None;
    BindType Type = BindType::None;
    std::string Display = "NONE";
    std::string Key = "NONE";
    UINT VK = UNUSABLE_VK;
    UINT Id = 0;

    BlankImGuiBind(UINT VK = UNUSABLE_VK, BindMode Mode = BindMode::Toggle) : Mode(Mode) {
        NextId += 1;
        this->Id = NextId;
        IdToBind[NextId] = this;

        if (VK != UNUSABLE_VK) {
            SetBindKey(this, VK);
        }
    }

    virtual void Update() {
        UpdateBind(this);
    }
    
    virtual ~BlankImGuiBind() = default;
};

inline bool UpdateKey(BlankImGuiBind* const Bind, const UINT VK) {
    if (Binds::Binding == &Bind->Id) {
        SetBindKey(Bind, VK);
        Binds::Binding = nullptr;
        return true;
    }
    
    return false;
}

class ImGuiBind : BlankImGuiBind {
public:
    Connection<UINT> OnPressConnection;
    bool Flag = false;

    ImGuiBind(UINT VK = UNUSABLE_VK, BindMode Mode = BindMode::Toggle)
    : BlankImGuiBind(VK, Mode) {
        Type = BindType::Normal;
        OnPressConnection = Binds::KeyPressed.connect([this](UINT VK_Key) {
            if (UpdateKey(this, VK_Key)) return;
            if (this->Mode == BindMode::Toggle && VK_Key == this->VK) {
                this->Flag = !this->Flag;
            }
        });
    }

    void Update() override {
        UpdateBind(this);

        switch (this->Mode) {
            case BindMode::Hold: {
                this->Flag = GetAsyncKeyState(this->VK);
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

    ~ImGuiBind() override {
        OnPressConnection.disconnect();
    }
};

class CallbackImGuiBind : public BlankImGuiBind {
public:
    Connection<UINT> OnPressConnection;
    std::function<void()> Callback;
    bool Holding;

    CallbackImGuiBind(
        std::function<void()> cb,
        BindMode BindMode = BindMode::None,
        UINT VK = UNUSABLE_VK
    ) : BlankImGuiBind(VK, BindMode),
        Callback(cb) {
        Type = BindType::Callback;
        OnPressConnection = Binds::KeyPressed.connect([this](const UINT VK_Key) {
            if (UpdateKey(this, VK_Key)) return;
            if (VK_Key != this->VK) return;
            if (this->Mode == BindMode::Hold) {
                this->Holding = true;
                auto HoldingConnection = std::make_shared<Connection<UINT>>(Connection<UINT>{});

                *HoldingConnection = Binds::KeyReleased.connect([this, HoldingConnection](const UINT VK_Key) {
                    if (this->VK != VK_Key) return;
                    HoldingConnection->disconnect();
                    this->Holding = false;
                });

                while (this->Holding) {
                    using namespace std::chrono;

                    const auto Start = high_resolution_clock::now();

                    this->Callback();

                    const auto Elapsed = high_resolution_clock::now() - Start;

                    if (Elapsed < milliseconds(10)) {
                        std::this_thread::sleep_for(milliseconds(10)
                         - duration_cast<milliseconds>(Elapsed));
                    }
                }
            } else {
                this->Callback();
            }
        });
    }

    ~CallbackImGuiBind() override {
        OnPressConnection.disconnect();
    }
};

class SliderCallbackImGuiBind : public BlankImGuiBind {
public:
    Connection<UINT> OnPressConnection;
    std::function<void(int)> Callback;
    std::string Label, Format;
    int Value, Min, Max;
    bool Destroying;
    bool Holding;

    SliderCallbackImGuiBind(
        std::function<void(int)> Callback,
        const std::string Label = "",
        const int DefaultValue = 0,
        const int Min = 0,
        const int Max = 100,
        const std::string Format = "%d",
        BindMode BindMode = BindMode::None
    ) : BlankImGuiBind(UNUSABLE_VK, BindMode),
        Callback(Callback),
        Label(Label),
        Value(DefaultValue),
        Min(Min),
        Max(Max),
        Format(Format) {
        Type = BindType::Callback;
        OnPressConnection = Binds::KeyPressed.connect([this](const UINT VK_Key) {
            if (UpdateKey(this, VK_Key)) return;
            if (VK_Key != this->VK) return;
            if (this->Mode == BindMode::Hold) {
                this->Holding = true;
                auto HoldingConnection = std::make_shared<Connection<UINT>>(Connection<UINT>{});

                *HoldingConnection = Binds::KeyReleased.connect([this, HoldingConnection](const UINT VK_Key) {
                    if (this->VK != VK_Key) return;
                    HoldingConnection->disconnect();
                    this->Holding = false;
                });

                while (this->Holding) {
                    using namespace std::chrono;

                    const auto Start = high_resolution_clock::now();

                    this->Callback(Value);

                    const auto Elapsed = high_resolution_clock::now() - Start;

                    if (Elapsed < milliseconds(10)) {
                        std::this_thread::sleep_for(milliseconds(10)
                         - duration_cast<milliseconds>(Elapsed));
                    }
                }
            } else {
                this->Callback(Value);
            }
        });
    }

    void Update() override {
        using namespace std::string_literals;
        
        const std::string IdEnding = "## " + std::to_string(Id);

        const std::string ConcatenatedButtonLabel = "-"s + IdEnding;
        Destroying = ImGui::Button(ConcatenatedButtonLabel.c_str());

        const std::string ConcatenatedLabel = Label + IdEnding;

        const float RegionWidth = ImGui::GetContentRegionAvail().x;
        const float LastElementWidth = ImGui::GetItemRectSize().x;
        const float DisplayWidth = ImGui::CalcTextSize(this->Display.c_str()).x;

        const ImGuiStyle Style = ImGui::GetStyle();
        const float InnerSpacingWidth = Style.ItemInnerSpacing.x;
        const float SpacingWidth = Style.ItemSpacing.x;

        ImGui::SameLine();
        ImGui::SetNextItemWidth(
            RegionWidth
            - LastElementWidth
            - SpacingWidth * 2
            - InnerSpacingWidth * 2
            - DisplayWidth
        );
        ImGui::SliderInt(ConcatenatedLabel.c_str(), &Value, Min, Max, Format.c_str());

        UpdateBind(this, true);
    }

    ~SliderCallbackImGuiBind() override {
        OnPressConnection.disconnect();
    }
};

using namespace Globals::MultiSliderCallbackImGuiBindSettings;
class MultiSliderCallbackImGuiBind : public BlankImGuiBind {
public:
    std::vector<SliderCallbackImGuiBind> CallbackBinds;
    const std::function<void(int)> Callback;

    bool WindowToggle = false;
    const int DefaultValue, Min, Max;
    const std::string Format;

    MultiSliderCallbackImGuiBind(
        std::function<void(int)> cb,
        int DefaultValue = 0,
        int Min = 0,
        int Max = 100,
        const std::string Format = "%d",
        BindMode BindMode = BindMode::None
    ) : BlankImGuiBind(UNUSABLE_VK, BindMode),
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

        std::string Window = "## " + std::to_string(this->Id);

        ImGui::SetNextWindowPos(ImGui::GetItemRectMax());

        if (SetNextWindowSize(
            ImGui::GetStyle(),
            WINDOW_WIDTH,
            std::clamp(static_cast<int>(CallbackBinds.size()) + 1, 1, 10),
            true
        )&& ImGui::Begin(
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

            const bool Clicked = ImGui::Button("+");
            if (Clicked && CallbackBinds.size() < CallbackBinds.capacity()) {
                CallbackBinds.emplace_back(Callback, "", DefaultValue, Min, Max, Format, Mode);
            }

            ImGui::End();
        }
    }
};