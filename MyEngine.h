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
#include"Header/Sound.h"
#include"Header/Input.h"
#include"Header/DebugCamera.h"
#include"Header/Camera.h"
#include"Header/CrashHandler.h"
#include"Header/Log.h"
#include"Header/ImGuiClass.h"

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

#include"Header/Wave.h"//波打ちアニメーション用
#include"Header/Balloon.h"

#pragma endregion


//class MyEngine
//{
//public:
//    void Initialize(int width, int height);
//    void BeginFrame();
//    void EndFrame();
//    void Finalize();
//
//    // 必要に応じてgetter
//    Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const { return device_; }
//    SwapChain& GetSwapChain() { return swapChain_; }
//    Window& GetWC() {
//        return window_;
//    };
//private:
//    LogFile logFile_;
//    Window window_;
//    DXGIFactory dxgiFactory_;
//    GPU gpu_;
//    Microsoft::WRL::ComPtr<ID3D12Device> device_;
//    CommandQueue commandQueue_;
//    CommandList commandList_;
//    SwapChain swapChain_;
//    RenderTargetView rtv_;
//#ifdef _DEBUG
//    DebugError debugError_;
//#endif
//    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;
//    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
//    Microsoft::WRL::ComPtr <ID3D12Resource> swapChainResources_[2] = { nullptr };
//
//    Fence fence_;
//    FenceEvent fenceEvent_;
//    D3D12_ROOT_PARAMETER rootParameters_[6] = {};
//};

