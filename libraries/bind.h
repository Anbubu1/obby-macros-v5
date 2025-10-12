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

    inline UINT NextId = 1;
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
    UINT Id = 1;

    BlankImGuiBind(
        const UINT VK = UNUSABLE_VK,
        const BindMode Mode = BindMode::Toggle
    ) : Id(Binds::NextId),
        Mode(Mode) {
        Binds::NextId += 1;
        IdToBind[Binds::NextId] = this;

        if (VK != UNUSABLE_VK) {
            SetBindKey(this, VK);
        }
    }

    virtual void Update() {
        UpdateBind(this);
    }
    
    virtual ~BlankImGuiBind() noexcept = default;
};

inline bool UpdateKey(BlankImGuiBind* const Bind, const UINT VK) {
    if (Binds::Binding == &Bind->Id && Globals::ImGuiShown) {
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

    ImGuiBind(const UINT VK = UNUSABLE_VK, const BindMode Mode = BindMode::Toggle)
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

    ~ImGuiBind() noexcept override {
        OnPressConnection.disconnect();
    }
};

class CallbackImGuiBind : public BlankImGuiBind {
private:
    Connection<UINT> OnPressConnection;
    bool Holding;

public:
    std::function<void()> Callback;

    CallbackImGuiBind(
        const std::function<void()> cb,
        const BindMode BindMode = BindMode::None,
        const UINT VK = UNUSABLE_VK
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

    ~CallbackImGuiBind() noexcept override {
        OnPressConnection.disconnect();
    }
};

class SliderCallbackImGuiBind : public BlankImGuiBind {
private:
    Connection<UINT> OnPressConnection;
    std::string Label, Format, ButtonLabel;
    float SemiFinalWidth;
    bool Holding = false;
    bool Cached = false;

public:
    std::function<void(int)> Callback;
    int Value, Min, Max;
    bool Destroying = false;

    SliderCallbackImGuiBind(
        const std::function<void(int)> Callback,
        const std::string Label = "",
        const int DefaultValue = 0,
        const int Min = 0,
        const int Max = 100,
        const std::string Format = "%d",
        const BindMode BindMode = BindMode::None
    ) : BlankImGuiBind(UNUSABLE_VK, BindMode),
        Callback(Callback),
        Label(Label + "##" + std::to_string(Binds::NextId)),
        ButtonLabel("-##" + std::to_string(Binds::NextId)),
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
class MultiSliderCallbackImGuiBind : public BlankImGuiBind {
public:
    std::vector<SliderCallbackImGuiBind> CallbackBinds;
    const std::function<void(int)> Callback;

    bool WindowToggle = false;
    const int DefaultValue, Min, Max;
    const std::string Format;

    MultiSliderCallbackImGuiBind(
        const std::function<void(int)> cb,
        const int DefaultValue = 0,
        const int Min = 0,
        const int Max = 100,
        const std::string Format = "%d",
        const BindMode BindMode = BindMode::None
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