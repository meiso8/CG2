
#include "../Header/PSO.h"

#include<cassert>

void PSO::Create(
    RootSignature& rootSignature,
    InputLayout& inputLayout,
    DxcCompiler& dxcCompiler,
    BlendState& blendState,
    //D3D12_RASTERIZER_DESC rasterizerDesc,
    RasterizerState& rasterizerState,
    DepthStencil& depthStencil,
    //D3D12_DEPTH_STENCIL_DESC depthStencilDesc,
    const Microsoft::WRL::ComPtr<ID3D12Device>& device) {

    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature.GetRootSignature().Get();//RootSignature
    graphicsPipelineStateDesc.InputLayout = inputLayout.GetDesc();//InputLayout
    graphicsPipelineStateDesc.VS = { dxcCompiler.GetVertexShaderBlob()->GetBufferPointer(),
   dxcCompiler.GetVertexShaderBlob()->GetBufferSize() };//VertexShader
    graphicsPipelineStateDesc.PS = { dxcCompiler.GetPixelShaderBlob()->GetBufferPointer(),
   dxcCompiler.GetPixelShaderBlob()->GetBufferSize() };//PixelShader
    graphicsPipelineStateDesc.BlendState = blendState.GetDesc();//BlendState
    graphicsPipelineStateDesc.RasterizerState = rasterizerState.GetDesc();//RasterizerState
    //書き込むRTVの情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    //利用するトポロジ（形状）のタイプ。三角形
    graphicsPipelineStateDesc.PrimitiveTopologyType =
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    //どのように画面に色を打ち込むかの設定（気にしなくていい）
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    //DepthStencilの設定
    graphicsPipelineStateDesc.DepthStencilState = depthStencil.GetDesc();
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    //実際に生成
    HRESULT hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
        IID_PPV_ARGS(&graphicsPipelineState_));
    assert(SUCCEEDED(hr));
}
