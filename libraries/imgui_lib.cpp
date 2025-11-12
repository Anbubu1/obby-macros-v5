#include <imgui_lib.hpp>

#include <windows.h>
#include <imgui.h>

#include <globals.hpp>

bool CreateDeviceD3D(HWND hWnd) {
    namespace DirectX = Globals::DirectX;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
#if defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };

    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
                                      createDeviceFlags, featureLevelArray, 2,
                                      D3D11_SDK_VERSION, &sd, &DirectX::g_pSwapChain,
                                      &DirectX::g_pd3dDevice, &featureLevel, &DirectX::g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CreateRenderTarget() {
    namespace DirectX = Globals::DirectX;

    ID3D11Texture2D* pBackBuffer;
    DirectX::g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    DirectX::g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &DirectX::g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    namespace DirectX = Globals::DirectX;

    if (DirectX::g_mainRenderTargetView) {
        DirectX::g_mainRenderTargetView->Release();
        DirectX::g_mainRenderTargetView = NULL;
    }
}

void CleanupDeviceD3D() {
    namespace DirectX = Globals::DirectX;
    
    CleanupRenderTarget();
    if (DirectX::g_pSwapChain) {
        DirectX::g_pSwapChain->Release();
        DirectX::g_pSwapChain = nullptr;
    }
    
    if (DirectX::g_pd3dDeviceContext) {
        DirectX::g_pd3dDeviceContext->Release();
        DirectX::g_pd3dDeviceContext = nullptr;
    }

    if (DirectX::g_pd3dDevice) {
        DirectX::g_pd3dDevice->Release();
        DirectX::g_pd3dDevice = nullptr;
    }
}

void SetImGuiScale(float scale) {
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(scale);

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = scale;
}

ImVec2 GetNextWindowSize(
    const ImGuiStyle& Style,
    const int WindowWidth,
    const int NonTextElements,
    const bool IgnoreTitleBar,
    const std::array<int, 2> TextElementsInfo,
    const int Separators
) {
    static const float TextLineHeight = ImGui::GetTextLineHeight();
    static const float FrameHeight = ImGui::GetFrameHeight();

    static const float WindowPadding = Style.WindowPadding.y;
    static const float ItemSpacing = Style.ItemSpacing.y;

    const float TitleBarHeight = (ImGui::GetFontSize() + Style.FramePadding.y * 2.0f) * !IgnoreTitleBar;

    const int LineWraps = TextElementsInfo[0];
    const int TextElements = TextElementsInfo[1];

    return ImVec2(
        static_cast<float>(WindowWidth),
        WindowPadding * 2
      + ItemSpacing * static_cast<float>(NonTextElements - 1)
      + TitleBarHeight
      + FrameHeight * static_cast<float>(NonTextElements)
      + static_cast<float>(TextElements + Separators) * ItemSpacing
      + static_cast<float>(LineWraps) * TextLineHeight
    );
}

ImVec2 GetNextWindowSize(
    const ImGuiStyle& Style,
    const WindowSizeParams Parameters,
    const int WindowFlags
) {
    return GetNextWindowSize(Style,
        Parameters.WindowWidth,
        Parameters.NonTextElements,
        WindowFlags & ImGuiWindowFlags_NoTitleBar,
        Parameters.TextElementsInfo,
        Parameters.Separators
    );
}

bool ComboFromStringVector(
    const char* Label,
    std::string& CurrentItem,
    const std::vector<std::string>& Items
) {
    int CurrentIndex = 0;
    for (size_t i = 0; i < Items.size(); ++i) {
        if (Items[i] == CurrentItem) [[likely]] {
            CurrentIndex = static_cast<int>(i);
            break;
        }
    }

    std::vector<const char*> ItemsCStr;
    ItemsCStr.reserve(Items.size());
    for (auto& s : Items)
        ItemsCStr.push_back(s.c_str());

    const bool Changed = ImGui::Combo(
        Label, &CurrentIndex,
        ItemsCStr.data(), static_cast<int>(ItemsCStr.size())
    );

    if (Changed && CurrentIndex >= 0 && CurrentIndex < static_cast<int>(Items.size()))
        CurrentItem = Items[static_cast<size_t>(CurrentIndex)];

    return Changed;
}