#pragma once

#include <windows.h>
#include <wininet.h>
#include <d3d11.h>
#include <fstream>
#include <string>
#include <vector>

#include <shellapi.h>
#include <shobjidl.h>
#include <shlobj.h>

LRESULT CALLBACK KeyboardProc(const int nCode, const WPARAM wParam, const LPARAM lParam);
LRESULT WINAPI WndProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam);

inline bool DownloadFile(const std::wstring& Url, const std::wstring& OutputPath) {
    HINTERNET hInternet = InternetOpenW(L"MyDownloader", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return false;

    HINTERNET hUrl = InternetOpenUrlW(hInternet, Url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hUrl) {
        InternetCloseHandle(hInternet);
        return false;
    }

    std::ofstream OutFile(OutputPath, std::ios::binary);
    if (!OutFile.is_open()) {
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        return false;
    }

    BYTE buffer[4096];
    DWORD BytesRead = 0;

    while (InternetReadFile(hUrl, buffer, sizeof(buffer), &BytesRead) && BytesRead > 0)
        OutFile.write(reinterpret_cast<char*>(buffer), BytesRead);

    OutFile.close();
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);

    return true;
}

inline std::vector<std::string> ListJsonFiles(const std::wstring& directory) {
    std::vector<std::string> Results;
    std::wstring SearchPattern = directory;
    if (!SearchPattern.empty() && SearchPattern.back() != L'\\' && SearchPattern.back() != L'/')
        SearchPattern += L'\\';
    SearchPattern += L"*.json";
    WIN32_FIND_DATAW FindData;
    HANDLE hFind = FindFirstFileW(SearchPattern.c_str(), &FindData);

    if (hFind == INVALID_HANDLE_VALUE) return Results;

    do {
        if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            const int Length = WideCharToMultiByte(
                CP_UTF8, 0, FindData.cFileName, -1,
                nullptr, 0, nullptr, nullptr
            );

            if (Length <= 1) continue;
            
            std::string filename(static_cast<size_t>(Length - 1), 0);
            WideCharToMultiByte(CP_UTF8, 0, FindData.cFileName, -1, filename.data(), Length, nullptr, nullptr);
            Results.push_back(filename);
        }
    } while (FindNextFileW(hFind, &FindData));

    FindClose(hFind);
    return Results;
}

inline bool OpenParentDirectoryAndSelectFile(const std::wstring& Path) {
    PIDLIST_ABSOLUTE PidList = ILCreateFromPathW(Path.c_str());
    if (!PidList) return false;

    HRESULT hResult = SHOpenFolderAndSelectItems(PidList, 0, nullptr, 0);
    ILFree(PidList);
    return SUCCEEDED(hResult);
}