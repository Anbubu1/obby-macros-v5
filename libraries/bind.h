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

bool UpdateBind(BlankImGuiBind* Bind);
void SetBindKey(BlankImGuiBind* Bind, UINT VK);
bool SetRightSideButton(BlankImGuiBind* Bind);

class BlankImGuiBind {
public:
    BindMode Mode = BindMode::None;
    BindType Type = BindType::None;
    const char* Display = "NONE";
    const char* Key = "NONE";
    UINT VK = UNUSABLE_VK;
    UINT Id = 0;

    BlankImGuiBind(UINT VK = UNUSABLE_VK, BindMode Mode = BindMode::Toggle) {
        NextId += 1;
        this->Id = NextId;
        this->Mode = Mode;
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
};

class CallbackImGuiBind : public BlankImGuiBind {
public:
    Connection<UINT> OnPressConnection;
    std::function<void()> Callback;
    BindType Type = BindType::Callback;

    CallbackImGuiBind(std::function<void()> cb, UINT VK = UNUSABLE_VK)
    : BlankImGuiBind(VK, BindMode::None), Callback(cb) {
        OnPressConnection = Binds::KeyPressed.connect([this](UINT VK_Key) {
            if (Binds::Binding == &this->Id) {
                SetBindKey(this, VK_Key);
                Binds::Binding = nullptr;
            } else if (VK_Key == this->VK) {
                this->Callback();
            }
        });
    }
};

class MultiSliderCallbackImGuiBind : public BlankImGuiBind {
public:
    static constexpr int MAX_CALLBACK_BINDS = Globals::MultiSliderCallbackImGuiBindSettings::MAX_CALLBACK_BINDS;

    std::vector<CallbackImGuiBind> CallbackBinds;
    const char* Display = "BINDS";
    const char* Key = "BINDS";

    bool WindowToggle = false;

    MultiSliderCallbackImGuiBind() : BlankImGuiBind(UNUSABLE_VK, BindMode::None) {
        CallbackBinds.reserve(MAX_CALLBACK_BINDS);
    }

    void Update() override {
        const bool Clicked = SetRightSideButton(this);
        if (Clicked) WindowToggle = !WindowToggle;

        if (!WindowToggle) return;

        POINT MousePosition;
        if (!GetCursorPos(&MousePosition)) return;
        ScreenToClient(hWnd, &MousePosition);

        std::string Window = "## " + std::to_string(this->Id);

        if (SetNextWindowSize(
            ImGui::GetStyle(),
            100,
            CallbackBinds.size(),
            true
        )&& ImGui::Begin(
            Window.c_str(),
            nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize
        )) {
            ImGui::End();
        }
    }
};