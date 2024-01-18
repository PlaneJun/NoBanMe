#pragma once
#include <vector>
#include <string>
#include <Windows.h>
#include <d3d11.h>
#include <D3DX11tex.h>

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dx11.lib")
#pragma comment(lib,"d3dcompiler.lib")
class render
{
private:
    static render* instance_;
    ID3D11Device* pd3dDevice_ ;
    ID3D11DeviceContext* pd3dDeviceContext_ ;
    IDXGISwapChain* pSwapChain_ ;
    ID3D11RenderTargetView* mainRenderTargetView_ ;
    HWND hwnd_;
    static uint32_t ResizeWidth_ , ResizeHeight_;
private:

    void CreateRenderTarget();

    bool CreateDeviceD3D(HWND hWnd);

    void CleanupRenderTarget();

    void CleanupDeviceD3D();

    ID3D11Device* GetDevice();

    IDXGISwapChain* GetSwapChain();

    static LRESULT WINAPI WndProcCallBack(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:

    static render* get_instasnce();

    HWND GetHwnd();

    bool CreatGui(const wchar_t* title, const wchar_t* classname, uint32_t w, uint32_t h, void(*cb)(float w, float h));

    void AddListBox(const char* uid, int& selected, uint8_t index, std::vector<std::string> headers, std::vector<std::vector<std::string>> text);

    void AddListBox(const char* uid, int& selected, std::vector<std::string> headers, std::vector<std::pair<std::string, std::string>> text);

    int DrawItemBlock(std::vector<std::string> items);

    void HelpMarker(const char* desc);

    ID3D11ShaderResourceView* DX11LoadTextureImageFromFile(const char* filepath);
};





