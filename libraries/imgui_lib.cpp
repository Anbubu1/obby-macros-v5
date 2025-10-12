#include <imgui_lib.h>

#include <wndproc.h>
#include <globals.h>

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