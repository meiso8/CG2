#pragma once

#include <fstream>
//ComPtr(コムポインタ)
#include<wrl.h>
#include<d3d12.h>

//Textureデータを読み込むためにDirectXTex.hをインクルード
#include"../externals/DirectXTex/DirectXTex.h"
//Textureの転送のために
#include"../externals/DirectXTex/d3dx12.h"

//テクスチャの読み込み関数
DirectX::ScratchImage LoadTexture(const std::string& filePath);

Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(
    const Microsoft::WRL::ComPtr<ID3D12Device>& device,
    const DirectX::TexMetadata& metadata);

//TextureResourceにデータを転送する　GPU
[[nodiscard]]
Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(
    const Microsoft::WRL::ComPtr<ID3D12Resource>& texture,
    const DirectX::ScratchImage& mipImages,
    const Microsoft::WRL::ComPtr<ID3D12Device>& device,
    const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
