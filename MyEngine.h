#pragma once

#pragma region //自作関数
#include"Header/Window.h"
#include"Header/CommandQueue.h"
#include"Header/CommandList.h"
#include"Header/DXGIFactory.h"
#include"Header/GPU.h"
#include"Header/SwapChain.h"
#include"Header/DescriptorHeap.h"
#include"Header/RenderTargetView.h"
#include"Header/GetCPUDescriptorHandle.h"
#include"Header/GetGPUDescriptorHandle.h"
#include"Header/DebugError.h"
#include"Header/TransitionBarrier.h"
#include"Header/Fence.h"
#include"Header/FenceEvent.h"
#include"Header/D3DResourceLeakChecker.h"
#include"Header/Depth.h"//StencilTextureの作成関数　奥行き
#include"Header/CompileShader.h"
#include"Header/BlendState.h"
#include"Header/RasterizerState.h"
#include"Header/PSO.h"
#include"Header/ViewPort.h"
#include"Header/ScissorRect.h"
#include"Header/Texture.h"
#include"Header/CreateBufferResource.h"
#include"Header/ShaderResourceView.h"
#include"Header/Model.h"
#include"Header/Sprite.h"
#include"Header/Line.h"
#include"Header/Sound.h"
#include"Header/Input.h"
#include"Header/DebugCamera.h"
#include"Header/Camera.h"
#include"Header/CrashHandler.h"
#include"Header/Log.h"
#include"Header/ImGuiClass.h"
#include"Header/InputLayout.h"
#include "Header/RootSignature.h"

#include"Header/Material.h"
#include"Header/VertexData.h"
#include"Header/DirectionalLight.h"
#include"Header/TransformationMatrix.h"
#include"Header/ModelData.h"

#include"Header/math/Normalize.h"
#include"Header/math/Transform.h"
#include"Header/math/MakeIdentity4x4.h"
#include"Header/math/MakeAffineMatrix.h"
#include"Header/math/Inverse.h"
#include"Header/math/MakePerspectiveFovMatrix.h"
#include"Header/math/MakeOrthographicMatrix.h"
#include"Header/math/Multiply.h"
#include"Header/math/SphericalCoordinate.h"

#include"Header/Wave.h"//波打ちアニメーション用
#include"Header/Balloon.h"

#include"Header/DrawGrid.h"
#include"Header/Cube.h"

#pragma endregion

class MyEngine {
public:
    void Create(int32_t clientWidth, int32_t clientHeight);
    void Update();
    void PreCommandSet();
    void PostCommandSet();
    void End();
    Window& GetWC() { return wc; };
    CommandList& GetCommandList() { return commandList; };
    ModelConfig& GetModelConfig() { return modelConfig_; };
    Microsoft::WRL::ComPtr<ID3D12Device>& GetDevice() { return device; };
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetSrvDescriptorHeap() { return srvDescriptorHeap; }
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
    BlendState blendState = {};
    RasterizerState rasterizerState = {};
    DepthStencil depthStencil = {};
    PSO pso = {};

    Microsoft::WRL::ComPtr <ID3D12Resource> waveResource = nullptr;
    Wave* waveData = nullptr;
    Microsoft::WRL::ComPtr <ID3D12Resource> expansionResource = nullptr;
    Balloon* expansionData = nullptr;
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
    ModelConfig modelConfig_ = {};
};

