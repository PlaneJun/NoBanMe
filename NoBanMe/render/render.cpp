#include <fstream>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui_theme.h>
#include "render.h"
#include "../resource.h"
#include "../IconsFontAwesome5.h"

render* render::instance_ = NULL;
uint32_t render::ResizeWidth_ = 0;
uint32_t render::ResizeHeight_ = 0;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void render::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    pSwapChain_->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    pd3dDevice_->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView_);
    pBackBuffer->Release();
}

bool render::CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
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
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain_, &pd3dDevice_, &featureLevel, &pd3dDeviceContext_);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain_, &pd3dDevice_, &featureLevel, &pd3dDeviceContext_);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void render::CleanupRenderTarget()
{
    if (mainRenderTargetView_) { mainRenderTargetView_->Release(); mainRenderTargetView_ = nullptr; }
}

void render::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (pSwapChain_) { pSwapChain_->Release(); pSwapChain_ = nullptr; }
    if (pd3dDeviceContext_) { pd3dDeviceContext_->Release(); pd3dDeviceContext_ = nullptr; }
    if (pd3dDevice_) { pd3dDevice_->Release(); pd3dDevice_ = nullptr; }
}

ID3D11Device* render::GetDevice()
{
    return pd3dDevice_;
}

IDXGISwapChain* render::GetSwapChain()
{
    return pSwapChain_;
}

LRESULT WINAPI render::WndProcCallBack(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED)
                return 0;
            ResizeWidth_ = lParam & 0xFFFF;
            ResizeHeight_ = (lParam & ~0xFFFF) >> 16;
            return 0;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

render* render::get_instasnce()
{
    if (instance_ == NULL)
        instance_ = new render();
    return instance_;
}

HWND render::GetHwnd()
{
    return hwnd_;
}

bool render::CreatGui(const wchar_t* title, const wchar_t* classname, uint32_t w, uint32_t h, void(*cb)(float w, float h))
{
    ResizeWidth_ = w;
    ResizeHeight_ = h;
    WNDCLASSEXW wc = {
        sizeof(wc),
        CS_CLASSDC,
        WndProcCallBack,
        0L,
        0L,
        GetModuleHandleA(nullptr),
        static_cast<HICON>(LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 256, 256, 0)),
        nullptr,
        nullptr,
        nullptr,
        classname,
        nullptr
    };

    RegisterClassExW(&wc);

    hwnd_ = CreateWindow(wc.lpszClassName,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        w, h,
        nullptr, nullptr,
        wc.hInstance,
        nullptr);

    if (!CreateDeviceD3D(hwnd_))
    {
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    RECT rect;
    GetClientRect(GetDesktopWindow(), &rect);
    rect.left = (rect.right / 2) - (w / 2);
    rect.top = (rect.bottom / 2) - (h / 2);
    MoveWindow(hwnd_, rect.left, rect.top, w, h, true);
    ShowWindow(hwnd_, SW_SHOWDEFAULT);
    UpdateWindow(hwnd_);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    std::string fontPath = "c:\\Windows\\Fonts\\msyh.ttc";
    std::ifstream inFile(fontPath);
    if (!inFile.is_open())
        fontPath = "c:\\Windows\\Fonts\\msyh.ttf";
    inFile.close();
    io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 15.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());

	float baseFontSize = 18.0f; // 13.0f is the size of the default font. Change to the font size you use.
	float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
	// merge in icons from Font Awesome
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphMinAdvanceX = iconFontSize;
	io.Fonts->AddFontFromFileTTF("./Font/fa-solid-900.ttf", iconFontSize, &icons_config, icons_ranges);


    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd_);
    ImGui_ImplDX11_Init(pd3dDevice_, pd3dDeviceContext_);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        if (ResizeWidth_ != 0 && ResizeHeight_ != 0)
        {
            CleanupRenderTarget();
            pSwapChain_->ResizeBuffers(0, ResizeWidth_, ResizeHeight_, DXGI_FORMAT_UNKNOWN, 0);
            //ResizeWidth_ = ResizeHeight_=0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (cb != nullptr)
            cb(ResizeWidth_, ResizeHeight_);

        // Rendering
        ImGui::Render();

        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        pd3dDeviceContext_->OMSetRenderTargets(1, &mainRenderTargetView_, nullptr);
        pd3dDeviceContext_->ClearRenderTargetView(mainRenderTargetView_, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        pSwapChain_->Present(1, 0); // Present with vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(hwnd_);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

void render::AddListBox(const char* uid, int& selected, uint8_t index, std::vector<std::string> headers, std::vector<std::vector<std::string>> text)
{
    ImGui::Columns(headers.size(), uid);
    for (int i = 0; i < headers.size(); i++)
    {
        ImGui::Text(headers[i].c_str());
        ImGui::NextColumn();
    }
    ImGui::Separator();
    for (int i = 0; i < text.size(); i++)
    {
        if (text[i].size() <= 0)
            break;

        for (int j = 0; j < text[i].size(); j++)
        {
            if (j == index)
            {
                if (ImGui::Selectable(text[i][j].c_str(), selected == i, ImGuiSelectableFlags_SpanAllColumns))
                    selected = i;
            }
            else
            {
                ImGui::Text(text[i][j].c_str());
            }

            ImGui::NextColumn();
        }
    }
}

void render::AddListBox(const char* uid, int& selected, std::vector<std::string> headers, std::vector<std::pair<std::string, std::string>> text)
{
    ImGui::Columns(headers.size(), uid);
    for (int i = 0; i < headers.size(); i++)
    {
        ImGui::Text(headers[i].c_str());
        ImGui::NextColumn();
    }
    ImGui::Separator();

    for (int i = 0; i < text.size(); i++)
    {
        if (ImGui::Selectable(text[i].first.c_str(), selected == i, ImGuiSelectableFlags_SpanAllColumns))
            selected = i;
        ImGui::NextColumn();
        ImGui::Text(text[i].second.c_str()); ImGui::NextColumn();
    }
}

int render::DrawItemBlock(std::vector<std::string> items)
{
    int select = -1;
    for (int i = 0; i < items.size(); i++)
    {
        if (ImGui::Selectable(items[i].c_str()))
        {
            select = i;
        }
    }
    return select;
};

void render::HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

ID3D11ShaderResourceView* render::DX11LoadTextureImageFromFile(const char* filepath)
{
    ID3D11Texture2D* pTexture2D = NULL;
    D3D11_TEXTURE2D_DESC dec{};

    HRESULT result;
    D3DX11_IMAGE_LOAD_INFO loadInfo;
    ZeroMemory(&loadInfo, sizeof(D3DX11_IMAGE_LOAD_INFO));
    loadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    loadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    loadInfo.MipLevels = D3DX11_DEFAULT; //这时会产生最大的mipmaps层。 
    loadInfo.MipFilter = D3DX11_FILTER_LINEAR;
    result = D3DX11CreateTextureFromFileA(pd3dDevice_, filepath, &loadInfo, NULL, (ID3D11Resource**)(&pTexture2D), NULL);
    if (result != S_OK)
        return NULL;
    pTexture2D->GetDesc(&dec);

    if (result != S_OK)
    {
        return NULL;
    }

    ID3D11ShaderResourceView* pFontTextureView = NULL;

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = dec.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    pd3dDevice_->CreateShaderResourceView(pTexture2D, &srvDesc, &pFontTextureView);
    return pFontTextureView;
}




