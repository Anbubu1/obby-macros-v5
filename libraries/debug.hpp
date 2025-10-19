#pragma once

#include <concepts>
#include <iostream>
#include <string>

template<typename T>
concept Numeric =
   (std::integral<T> && !std::same_as<T, bool>) ||
    std::floating_point<T>;

template<typename T>
concept String =
    std::same_as<std::string, bool> ||
    std::same_as<std::string_view, bool> ||
    std::same_as<const char*, bool>;

namespace Debug {
    inline std::string ToString(const bool Boolean) {
        return &"false\0true"[Boolean * 6];
    }

    template<String T>
    inline void Print(const T String) {
        std::cout << String << '\n';
    }

    template<String string, size_t N>
    inline void Print(const std::array<string, N>& Array, const string Concat) {
        std::string Out = "";
        Out.reserve(N * (Concat.size() + 8));

        for (size_t i = 0; i < Array.size(); ++i) {
            Out += Array[i];
            Out.append(Concat.data(), Concat.size() * static_cast<size_t>(i != N - 1));
        }

        Debug::Print(Out);
    }

    template<Numeric T>
    inline void Print(const T Number) {
        std::cout << std::to_string(Number) << '\n';
    }

    inline void Print(const bool Boolean) {
        std::cout << Debug::ToString(Boolean) << '\n';
    }
}