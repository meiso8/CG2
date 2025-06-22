#pragma once

#include <Windows.h>  

//ComPtr(コムポインタ)
#include<wrl.h>
#include<dxgi1_6.h>
#include<d3d12.h>

#pragma region //ImGuiのincludeと関数の外部宣言
#ifdef _DEBUG

#include"../externals/imgui/imgui.h"
#include"../externals/imgui/imgui_impl_dx12.h"
#include"../externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif

class CommandList;

class ImGuiClass
{
public:
    void Initialize(HWND hWnd,
        const Microsoft::WRL::ComPtr<ID3D12Device>& device,
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc,
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc,
        const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap);

    void FrameStaert();

    void Render();

    void DrawImGui(CommandList& commandList);

    void ShutDown();

};

