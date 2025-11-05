#pragma once

#include <general.hpp>
#include <json.hpp>

void SaveConfig();
void LoadConfig();
void CreateConfig(std::string Name);
bool RemoveConfig(std::string Name = "");