#pragma once

#include <windows.h>
#include <fstream>

#include <json.hpp>

#include <string>

constexpr std::string ToUpper(std::string_view StringView) {
    std::string Result;
    for (char c : StringView)
        Result.push_back((c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c);
    return Result;
}

inline std::string GetReadableKeyName(const UINT VK) {
    using namespace std::string_literals;

    UINT ScanCode = MapVirtualKeyEx(VK, MAPVK_VK_TO_VSC, GetKeyboardLayout(0));

    switch (VK) {
        case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
        case VK_PRIOR: case VK_NEXT: case VK_END: case VK_HOME:
        case VK_INSERT: case VK_DELETE:
        case VK_DIVIDE: case VK_NUMLOCK:
            ScanCode |= 0x100;
            break;
    }

    LONG lParam = ScanCode << 16;

    char Name[64];
    if (GetKeyNameTextA(lParam, Name, sizeof(Name)) > 0) { [[likely]]
        return std::string(Name);
    } else {
        return "Unknown"s;
    }
}

inline std::string ToUpperWin(const std::string& Input) {
    std::string Result = Input;
    if (!Result.empty()) CharUpperA(&Result[0]);
    return Result;
}

inline bool IsKeyHeld(const int VK) {
    return (GetAsyncKeyState(VK) & 0x8000) != 0;
}

template <typename T>
inline T JsonIndexDefault(nlohmann::json& Json, const std::string& Key, const T& DefaultValue) {
    if (!Json.contains(Key)) { [[unlikely]]
        Json[Key] = DefaultValue;
        return DefaultValue;
    }

    try {
        return Json.at(Key).get<T>();
    } catch (const nlohmann::json::type_error&) {
        return DefaultValue;
    }
}

inline void ReadJson(const std::filesystem::path Path, nlohmann::json* Value) {
    std::ifstream File(Path);
    std::stringstream Buffer;
    Buffer << File.rdbuf();
    std::string Contents = Buffer.str();
    if (!Contents.empty()) [[unlikely]]
        *Value = nlohmann::json::parse(Contents);
}

inline bool WriteIfJsonNoExist(const std::filesystem::path Path, const std::string Value, const std::string Error) {
    if (!std::filesystem::exists(Path)) [[unlikely]] {
        std::ofstream File(Path);
        if (!File) [[unlikely]]
            throw std::runtime_error(Error);
        File << Value;
        File.close();
        return true;
    }

    return false;
}