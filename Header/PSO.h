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

    enum PSOType {
        TRIANGLE,
        LINE,
        POINT,
        NONE_TEX,
        PSO_MAX
    };

    void Create(
        RootSignature& rootSignature,
        InputLayout& inputLayout,
        DxcCompiler& dxcCompiler,
        BlendState& blendState,
        RasterizerState& rasterizerState,
        DepthStencil& depthStencil,
        const Microsoft::WRL::ComPtr<ID3D12Device>& device);

    Microsoft::WRL::ComPtr <ID3D12PipelineState>& GetGraphicsPipelineState(PSOType type) {
        return graphicsPipelineState_[type];
            ;
    }
private:
    Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState_[PSO_MAX] = { nullptr };
};