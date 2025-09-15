#include <general.h>

#include <algorithm>
#include <iostream>
#include <cstring>
#include <cctype>

char* to_upper(const char* input) {
    size_t len = std::strlen(input);
    char* result = new char[len + 1];

    for (size_t i = 0; i < len; ++i) {
        result[i] = std::toupper(static_cast<unsigned char>(input[i]));
    }
    result[len] = '\0';

    return result;
}

std::string to_upper(const std::string& input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

char* GetReadableKeyNameChar(UINT vk) {
    thread_local static char nameBuf[64] = {};

    UINT scanCode = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC);

    switch (vk) {
        case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
        case VK_PRIOR: case VK_NEXT: case VK_END: case VK_HOME:
        case VK_INSERT: case VK_DELETE: case VK_DIVIDE:
            scanCode |= 0x100;
            break;
    }

    LONG lParamForName = (scanCode << 16);

    if (GetKeyNameTextA(lParamForName, nameBuf, sizeof(nameBuf))) {
        return nameBuf;
    }

    snprintf(nameBuf, sizeof(nameBuf), "VK_%u", vk);
    return nameBuf;
}

std::string GetReadableKeyName(UINT vk) {
    UINT scanCode = MapVirtualKeyEx(vk, MAPVK_VK_TO_VSC, GetKeyboardLayout(0));

    switch (vk) {
        case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
        case VK_PRIOR: case VK_NEXT: case VK_END: case VK_HOME:
        case VK_INSERT: case VK_DELETE:
        case VK_DIVIDE: case VK_NUMLOCK:
            scanCode |= 0x100;
            break;
    }

    LONG lParam = scanCode << 16;

    char name[64];
    if (GetKeyNameTextA(lParam, name, sizeof(name)) > 0) {
        return std::string(name);
    } else {
        return "Unknown";
    }
}


std::string ToUpperWin(const std::string& input) {
    std::string result = input;
    if (!result.empty()) CharUpperA(&result[0]);
    return result;
}