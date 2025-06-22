#pragma once

#include<d3d12.h>
//ComPtr(コムポインタ)
#include<wrl.h>

//Textureデータを読み込むためにDirectXTex.hをインクルード
#include"../externals/DirectXTex/DirectXTex.h"
////Textureの転送のために
//#include"../externals/DirectXTex/d3dx12.h"

class ShaderResourceView
{
public:
    void Create(
        const DirectX::TexMetadata& metadata,
        const Microsoft::WRL::ComPtr<ID3D12Resource>& textureResource,
        uint32_t index,
        const Microsoft::WRL::ComPtr<ID3D12Device>& device,
        const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap,
        uint32_t descriptorSize
    );

    D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU()const {
        return textureSrvHandleGPU_
            ;
    }
private:
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_;
};

