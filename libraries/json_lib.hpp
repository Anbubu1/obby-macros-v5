#pragma once

#include <globals.hpp>
#include <json.hpp>

inline void SaveConfig() {
    nlohmann::json& FloatSliderFlags = Globals::JsonConfig["FloatSliderFlags"];
    nlohmann::json& IntSliderFlags = Globals::JsonConfig["IntSliderFlags"];
    nlohmann::json& BooleanFlags = Globals::JsonConfig["BooleanFlags"];

    for (const auto& pair : Globals::BooleanFlags)
        BooleanFlags[pair.first] = pair.second.load();

    for (const auto& pair : Globals::IntSliderFlags)
        IntSliderFlags[pair.first] = pair.second.load();

    for (const auto& pair : Globals::FloatSliderFlags)
        FloatSliderFlags[pair.first] = pair.second.load();
}

inline void LoadConfig() {    
    for (auto& [Key, Value] : Globals::JsonConfig["BooleanFlags"].items())
        Globals::BooleanFlags[Key].store(Value);

    for (auto& [Key, Value] : Globals::JsonConfig["IntSliderFlags"].items())
        Globals::IntSliderFlags[Key].store(Value);

    for (auto& [Key, Value] : Globals::JsonConfig["FloatSliderFlags"].items())
        Globals::FloatSliderFlags[Key].store(Value);
}