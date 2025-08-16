#pragma once

#pragma region //自作関数
#include"Source/Window.h"
#include"Source/CommandQueue.h"
#include"Source/CommandList.h"
#include"Source/DXGIFactory.h"
#include"Source/GPU.h"
#include"Source/SwapChain.h"
#include"Source/DescriptorHeap.h"
#include"Source/RenderTargetView.h"
#include"Source/GetCPUDescriptorHandle.h"
#include"Source/GetGPUDescriptorHandle.h"
#include"Source/DebugError.h"
#include"Source/TransitionBarrier.h"
#include"Source/Fence.h"
#include"Source/FenceEvent.h"
#include"Source/D3DResourceLeakChecker.h"
#include"Source/Depth.h"//StencilTextureの作成関数　奥行き
#include"Source/CompileShader.h"
#include"Source/BlendState.h"
#include"Source/RasterizerState.h"
#include"Source/PSO.h"
#include"Source/ViewPort.h"
#include"Source/ScissorRect.h"
#include"Source/Texture.h"
#include"Source/CreateBufferResource.h"
#include"Source/ShaderResourceView.h"
#include"Source/Model.h"
#include"Source/Sprite.h"
#include"Source/SphereMesh.h"
#include"Source/LineMesh.h"
#include"Source/Sound.h"
#include"Source/Input.h"
#include"Source/DebugCamera.h"
#include"Source/Camera.h"
#include"Source/CrashHandler.h"
#include"Source/Log.h"
#include"Source/ImGuiClass.h"
#include"Source/InputLayout.h"
#include "Source/RootSignature.h"

#include"Source/FPSCounter.h"

#include"Source/Material.h"
#include"Source/VertexData.h"
#include"Source/DirectionalLight.h"
#include"Source/TransformationMatrix.h"
#include"Source/ModelData.h"

#include"Source/math/Normalize.h"
#include"Source/math/Transform.h"
#include"Source/math/MakeIdentity4x4.h"
#include"Source/math/MakeAffineMatrix.h"
#include"Source/math/Inverse.h"
#include"Source/math/MakePerspectiveFovMatrix.h"
#include"Source/math/MakeOrthographicMatrix.h"
#include"Source/math/MakeRotateMatrix.h"
#include"Source/math/Multiply.h"
#include"Source/math/SphericalCoordinate.h"
#include"Source/math/Lerp.h"

#include"Source/DrawGrid.h"
#include"Source/Cube.h"

#include"Source/DebugUI.h"

#pragma endregion

class MyEngine {
public:
    void Create(const std::wstring& title, int32_t clientWidth, int32_t clientHeight);
    void Update();
    void PreCommandSet(Vector4& color);
    void PostCommandSet();
    void End();
    Window& GetWC() { return wc; };
    CommandList& GetCommandList() { return commandList; };
    ModelConfig& GetModelConfig(size_t index) { return modelConfig_[index]; };
    Microsoft::WRL::ComPtr<ID3D12Device>& GetDevice() { return device; };
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetSrvDescriptorHeap() { return srvDescriptorHeap; }
    DirectionalLight& GetDirectionalLightData() { return *directionalLightData; }
private:

    int32_t clientWidth_ = 1280;
    int32_t clientHeight_ = 720;

    D3DResourceLeakChecker leakCheck = {};
    LogFile logFile = {};
    std::ofstream logStream;
    Window wc = {};
    DXGIFactory dxgiFactory = {};
    GPU gpu = {};
    Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;

#ifdef _DEBUG
    DebugError debugError = {};
#endif
    CommandQueue commandQueue = {};
    CommandList commandList;
    SwapChain swapChainClass;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;
    //ゲームに一つだけ
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = nullptr;
    Microsoft::WRL::ComPtr <ID3D12Resource> swapChainResources[2] = { nullptr };
    RenderTargetView rtvClass = {};
    Fence fence = {};
    FenceEvent fenceEvent = {};
    DxcCompiler dxcCompiler = {};
    RootSignature rootSignature = {};
    InputLayout inputLayout = {};
    BlendState blendState[2] = {};
    RasterizerState rasterizerState[2] = {};
    DepthStencil depthStencil = {};
    PSO pso[3] = {};
    Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilResource = nullptr;
    Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResource = nullptr;
    DirectionalLight* directionalLightData = nullptr;
    D3D12_VIEWPORT viewport = {};
    D3D12_RECT scissorRect = {};
#ifdef _DEBUG
    ImGuiClass imGuiClass = {};
#endif // _DEBUG

    TransitionBarrier barrier = {};
    ModelConfig modelConfig_[3] = {};
};

