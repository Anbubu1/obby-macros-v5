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
    template<String T>
    inline void Print(const T String) {
        std::cout << String << std::endl;
    };

    template<Numeric T>
    inline void Print(const T Number) {
        std::cout << std::to_string(Number) << std::endl;
    };

    inline void Print(const bool Boolean) {
        std::cout << (&"false\0true"[Boolean * 6]) << '\n';
    };

    inline std::string ToString(const bool Boolean) {
        return &"false\0true"[Boolean * 6];
    };
}