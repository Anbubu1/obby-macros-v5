#pragma once

#include "globals.hpp"
#include <globals.hpp>
#include <general.hpp>
#include <json.hpp>

#include <filesystem>

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

    std::ofstream ConfigFile(Globals::ConfigPath);
    const std::string JsonDump = Globals::JsonConfig.dump();
    ConfigFile.write(JsonDump.c_str(), JsonDump.length());
    ConfigFile.close();
}

inline void LoadConfig() {
    for (auto& [Key, Value] : Globals::JsonConfig["BooleanFlags"].items())
        Globals::BooleanFlags[Key].store(Value);

    for (auto& [Key, Value] : Globals::JsonConfig["IntSliderFlags"].items())
        Globals::IntSliderFlags[Key].store(Value);

    for (auto& [Key, Value] : Globals::JsonConfig["FloatSliderFlags"].items())
        Globals::FloatSliderFlags[Key].store(Value);
}

inline void CreateConfig(std::string Name) {
    if (!Name.ends_with(".json"))
        Name += ".json";

    Globals::ConfigPath = Globals::ConfigFolderPath / Name;

    std::ofstream File(Globals::ConfigPath);
    if (!File)
        throw std::runtime_error(std::format("Failed to create {}!", Name));
    File << Globals::DefaultConfigString;
    File.close();

    Globals::JsonConfigPaths = ListJsonFiles(Globals::ConfigFolderPath);
}

inline bool RemoveConfig(std::string Name = "") {
    const bool UseCurrent = Name.empty();

    if (UseCurrent) {
        Name = Globals::CurrentConfigName;
        if (Name == Globals::DEFAULT_CONFIG_NAME) return false;
    } else {
        if (!Name.ends_with(".json")) Name += ".json";
        if (Name == Globals::DEFAULT_CONFIG_NAME || Name == Globals::CurrentConfigName)
            return false;
    }

    const std::filesystem::path ConfigPath = Globals::ConfigFolderPath / Name;

    std::error_code ErrorCode;
    if (!std::filesystem::remove(ConfigPath, ErrorCode) && ErrorCode)
        throw std::runtime_error(std::format("Failed to delete file: {}", ErrorCode.message()));

    if (UseCurrent) Globals::CurrentConfigName = Globals::DEFAULT_CONFIG_NAME;

    Globals::JsonConfigPaths = ListJsonFiles(Globals::ConfigFolderPath);

    return true;
}