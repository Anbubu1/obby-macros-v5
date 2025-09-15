#pragma once

#include <unordered_map>
#include <functional>
#include <iostream>
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
    inline Signal<UINT> KeyPressed;
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

bool UpdateBind(BlankImGuiBind* Bind, bool NoDummy = false);
void SetBindKey(BlankImGuiBind* Bind, UINT VK);
bool SetRightMostButton(BlankImGuiBind* Bind, bool NoDummy = false);

class BlankImGuiBind {
public:
    BindMode Mode = BindMode::None;
    BindType Type = BindType::None;
    const char* Display = "NONE";
    const char* Key = "NONE";
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

class ImGuiBind : BlankImGuiBind {
public:
    Connection<UINT> OnPressConnection;
    bool Flag = false;

    ImGuiBind(UINT VK = UNUSABLE_VK, BindMode Mode = BindMode::Toggle)
    : BlankImGuiBind(VK, Mode) {
        Type = BindType::Normal;
        OnPressConnection = Binds::KeyPressed.connect([this](UINT VK_Key) {
            if (Binds::Binding == &this->Id) {
                SetBindKey(this, VK_Key);
                Binds::Binding = nullptr;
            } else if (this->Mode == BindMode::Toggle && VK_Key == this->VK) {
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

    CallbackImGuiBind(
        std::function<void()> cb,
        UINT VK = UNUSABLE_VK
    ) : BlankImGuiBind(VK, BindMode::None),
        Callback(cb) {
        Type = BindType::Callback;
        OnPressConnection = Binds::KeyPressed.connect([this](UINT VK_Key) {
            if (Binds::Binding == &this->Id) {
                SetBindKey(this, VK_Key);
                Binds::Binding = nullptr;
            } else if (VK_Key == this->VK) {
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
    const char *Label, *Format;
    int Value, Min, Max;
    bool Destroying;

    SliderCallbackImGuiBind(
        std::function<void(int)> Callback,
        const char* Label = "",
        int DefaultValue = 0,
        int Min = 0,
        int Max = 100,
        const char* Format = "%d"
    ) : BlankImGuiBind(UNUSABLE_VK, BindMode::None),
        Callback(Callback),
        Label(Label),
        Value(DefaultValue),
        Min(Min),
        Max(Max),
        Format(Format) {
        Type = BindType::Callback;
        OnPressConnection = Binds::KeyPressed.connect([this](UINT VK_Key) {
            if (Binds::Binding == &this->Id) {
                SetBindKey(this, VK_Key);
                Binds::Binding = nullptr;
            } else if (VK_Key == this->VK) {
                this->Callback(Value);
            }
        });
    }

    void Update() override {
        const std::string IdEnding = "## " + std::to_string(Id);

        const std::string ConcatenatedButtonLabel = std::string("-") + IdEnding;
        Destroying = ImGui::Button(ConcatenatedButtonLabel.c_str());

        const std::string ConcatenatedLabel = std::string(Label) + IdEnding;

        const float RegionWidth = ImGui::GetContentRegionAvail().x;
        const float LastElementWidth = ImGui::GetItemRectSize().x;
        const float DisplayWidth = ImGui::CalcTextSize(this->Display).x;

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
        ImGui::SliderInt(ConcatenatedLabel.c_str(), &Value, Min, Max, Format);

        UpdateBind(this, true);
    }

    ~SliderCallbackImGuiBind() override {
        OnPressConnection.disconnect();
    }
};

class MultiSliderCallbackImGuiBind : public BlankImGuiBind {
public:
    static constexpr int MAX_CALLBACK_BINDS = Globals::MultiSliderCallbackImGuiBindSettings::MAX_CALLBACK_BINDS;

    std::vector<SliderCallbackImGuiBind> CallbackBinds;
    std::function<void(int)> Callback;

    bool WindowToggle = false;

    MultiSliderCallbackImGuiBind(std::function<void(int)> cb)
    : BlankImGuiBind(UNUSABLE_VK, BindMode::None), Callback(cb) {
        Type = BindType::Multi;
        Display = "BINDS";
        Key = "BINDS";
        CallbackBinds.reserve(MAX_CALLBACK_BINDS);
    }

    void Update() override {
        const bool Clicked = SetRightMostButton(this);
        if (Clicked) WindowToggle = !WindowToggle;
        if (!WindowToggle) return;

        std::string Window = "## " + std::to_string(this->Id);

        ImGui::SetNextWindowPos(ImGui::GetItemRectMax());

        if (SetNextWindowSize(
            ImGui::GetStyle(),
            150,
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
                CallbackBinds.emplace_back(Callback);
            }

            ImGui::End();
        }
    }
};