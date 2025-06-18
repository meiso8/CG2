#pragma once

#include"../Header/CommandList.h"  
#include"../Header/ModelData.h"  
#include"../Header/PSO.h"  
#include"../Header/ShaderResourceView.h"  

class Sprite
{
public:
    void Create(
        const Microsoft::WRL::ComPtr<ID3D12Device>& device);

    void Draw(
        CommandList& commandList,
        const Microsoft::WRL::ComPtr <ID3D12Resource>& materialResource,
        const Microsoft::WRL::ComPtr <ID3D12Resource>& transformationMatrixResource,
        ShaderResourceView& srv
    );

    void DrawCall(CommandList& commandList);
private:
    void CreateVertex(const Microsoft::WRL::ComPtr<ID3D12Device>& device);
    void CreateIndexResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device);
private:
    Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource_;
    //頂点バッファビューを作成する
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    VertexData* vertexData_ = nullptr;
    D3D12_INDEX_BUFFER_VIEW  indexBufferView_{};
    Microsoft::WRL::ComPtr <ID3D12Resource> indexResource_;
    uint32_t* indexData_ = nullptr;
};

