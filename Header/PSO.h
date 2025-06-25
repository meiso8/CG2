#pragma once

#include<d3d12.h>
#include<dxgi1_6.h>
#include<dxcapi.h>
//ComPtr(コムポインタ)
#include<wrl.h>

class PSO {
public:
    void Create(
        const Microsoft::WRL::ComPtr <ID3D12RootSignature>& rootSignature,
        D3D12_INPUT_LAYOUT_DESC inputLayoutDesc,
        const Microsoft::WRL::ComPtr <IDxcBlob>& vertexShaderBlob,
        const Microsoft::WRL::ComPtr <IDxcBlob>& pixelShaderBlob,
        D3D12_BLEND_DESC blendDesc,
        D3D12_RASTERIZER_DESC rasterizerDesc,
        D3D12_DEPTH_STENCIL_DESC depthStencilDesc,
        const Microsoft::WRL::ComPtr<ID3D12Device>& device);
    Microsoft::WRL::ComPtr <ID3D12PipelineState>& GetGraphicsPipelineState() {
        return graphicsPipelineState_
            ;
    }
private:
    Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState_ = nullptr;
};