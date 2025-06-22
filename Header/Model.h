#pragma once  
#include"../Header/CommandList.h"  
#include"../Header/ModelData.h"  
#include"../Header/PSO.h"  
#include"../Header/ShaderResourceView.h"  
#include"../Header/MaterialResource.h"
#include"../Header/Camera.h"
#include"../Header/TransformationMatrix.h"

class Model
{
public:

    Model() : camera_(nullptr), wvpDate_(nullptr) {}

    void Create(
        const std::string& directoryPath,
        const std::string& filename,
        Camera& camera,
        const Microsoft::WRL::ComPtr<ID3D12Device>& device,
        CommandList& commandList,
        const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap,
        const uint32_t& descriptorSizeSRV);

    void CreateWorldVP(const Microsoft::WRL::ComPtr<ID3D12Device>& device);

    void Update();

    void InitTraslate();

    void Draw(
        CommandList& commandList
    );

    void DrawCall(CommandList& commandList);

    Material* Getmaterial() { return materialResource_.GetMaterial(); };

    Transform& GetTransformRef() {
        return transform_;
    };

    VertexData* GetVertexData() {
        return vertexData_;
    }

private:
    MaterialResource materialResource_;
    ShaderResourceView srv_;

    Camera* camera_;

    Microsoft::WRL::ComPtr <ID3D12Resource> wvpResource_;
    TransformationMatrix* wvpDate_;

    //三角形の座標
    Transform transform_ = { 0.0f };
    //三角形の行列
    Matrix4x4 worldMatrix_ = { 0.0f };
    //WVpMatrixを作る
    Matrix4x4 worldViewProjectionMatrix_ = { 0.0f };

    ModelData modelData_;

    //頂点バッファビューを作成する
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    Microsoft::WRL::ComPtr<ID3D12Resource>vertexResource_;
    VertexData* vertexData_ = nullptr;

    DirectX::ScratchImage mipImages_;
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_;
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource_;

};
