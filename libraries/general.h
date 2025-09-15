#include <string>

#include <windows.h>

char* to_upper(const char* input);
std::string to_upper(const std::string& input);
char* GetReadableKeyNameChar(UINT vk);
std::string GetReadableKeyName(UINT vk);
std::string ToUpperWin(const std::string& input);