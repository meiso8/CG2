
#include "../Header/PSO.h"

#include<cassert>

void PSO::Create(
    const Microsoft::WRL::ComPtr <ID3D12RootSignature>& rootSignature,
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc,
    const Microsoft::WRL::ComPtr <IDxcBlob>& vertexShaderBlob,
    const Microsoft::WRL::ComPtr <IDxcBlob>& pixelShaderBlob,
    D3D12_BLEND_DESC blendDesc,
    D3D12_RASTERIZER_DESC rasterizerDesc,
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc,
    const Microsoft::WRL::ComPtr<ID3D12Device>& device) {

    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();//RootSignature
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;//InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
    vertexShaderBlob->GetBufferSize() };//VertexShader
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
    pixelShaderBlob->GetBufferSize() };//PixelShader
    graphicsPipelineStateDesc.BlendState = blendDesc;//BlendState
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;//RasterizerState
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
    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    //実際に生成
    HRESULT hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
        IID_PPV_ARGS(&graphicsPipelineState_));
    assert(SUCCEEDED(hr));
}
