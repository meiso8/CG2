#pragma once

#include<d3d12.h>
#include<dxgi1_6.h>
#include<dxcapi.h>
//ComPtr(コムポインタ)
#include<wrl.h>
#include"../Header/RootSignature.h"
#include"../Header/InputLayout.h"
#include"../Header/CompileShader.h"
#include"../Header/BlendState.h"
#include"../Header/RasterizerState.h"
#include"../Header/Depth.h"

class PSO {
public:
    void Create(
        RootSignature& rootSignature,
        InputLayout& inputLayout,
        //D3D12_INPUT_LAYOUT_DESC inputLayoutDesc,
        DxcCompiler& dxcCompiler,
        BlendState& blendState,
  /*      D3D12_BLEND_DESC blendDesc,*/
   /*     D3D12_RASTERIZER_DESC rasterizerDesc,*/
        RasterizerState& rasterizerState,
        DepthStencil& depthStencil,
     /*   D3D12_DEPTH_STENCIL_DESC depthStencilDesc,*/
        const Microsoft::WRL::ComPtr<ID3D12Device>& device);
    Microsoft::WRL::ComPtr <ID3D12PipelineState>& GetGraphicsPipelineState() {
        return graphicsPipelineState_
            ;
    }
private:
    Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState_ = nullptr;
};