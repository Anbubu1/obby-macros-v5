#pragma once

#include <concepts>
#include <iostream>
#include <string>

template<typename T>
concept Numeric =
   (std::integral<T> && !std::same_as<T, bool>) ||
    std::floating_point<T>;

template<typename T>
concept ReadOnlyString =
    std::same_as<std::string_view, bool> ||
    std::same_as<const char*, bool>;

template<typename T>
concept String =
    std::same_as<std::string, bool> ||
    std::same_as<std::string_view, bool> ||
    std::same_as<const char*, bool>;

namespace Debug {
    inline std::string_view ToString(const bool Boolean) {
        return &"false\0true"[Boolean * 6];
    }

    template<String T>
    inline void Print(const T& String) {
        std::cout << String << '\n';
    }

    template<String string, size_t len>
    constexpr std::string BuildStringFromArray(const std::array<string, len>& Array, const string& Concat) {
        std::string Out;
        std::string_view ConcatView = Concat;
        Out.reserve(len * (ConcatView.size() + 8));

        for (size_t i = 0; i < len; ++i) {
            Out.append(std::string_view(Array[i]));
            if (i != len - 1)
                Out.append(ConcatView);
        }

        return Out;
    }

    template<String string, size_t len>
    inline void Print(const std::array<string, len>& Array, const string& Concat) {
        Debug::Print(Debug::BuildStringFromArray(Array, Concat));
    }

    template<Numeric T>
    inline void Print(const T Number) {
        std::cout << std::to_string(Number) << '\n';
    }

    inline void Print(const bool Boolean) {
        std::cout << Debug::ToString(Boolean) << '\n';
    }
}