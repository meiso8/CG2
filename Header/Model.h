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
    Model(Camera& camera, CommandList& commandList, D3D12_VIEWPORT& viewport,
        D3D12_RECT& scissorRect,
        const Microsoft::WRL::ComPtr<ID3D12RootSignature>& rootSignature,
        PSO& pso);

    void Create(
        const std::string& directoryPath,
        const std::string& filename,    
        const Microsoft::WRL::ComPtr<ID3D12Device>& device,    
        const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap);

    void CreateWorldVP(const Microsoft::WRL::ComPtr<ID3D12Device>& device);

    void Update();

    void InitTraslate();

    void PreDraw();
    void Draw();

    void DrawCall();

    Material* Getmaterial() { return materialResource_.GetMaterial(); };

    Transform& GetTransformRef() {
        return transform_;
    };

    VertexData* GetVertexData() {
        return vertexData_;
    }

private:
    ShaderResourceView srv_;
    CommandList* commandList_ = nullptr;
    D3D12_VIEWPORT* viewport_ = nullptr;
    D3D12_RECT* scissorRect_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    PSO* pso_ = nullptr;

    Camera* camera_ = nullptr;

    Microsoft::WRL::ComPtr <ID3D12Resource> wvpResource_;
    TransformationMatrix* wvpDate_ = nullptr;

    //三角形の座標
    Transform transform_ = { 0.0f };
    //三角形の行列
    Matrix4x4 worldMatrix_ = { 0.0f };
    //WVpMatrixを作る
    Matrix4x4 worldViewProjectionMatrix_ = { 0.0f };

    MaterialResource materialResource_;
    ModelData modelData_;

    //頂点バッファビューを作成する
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    Microsoft::WRL::ComPtr<ID3D12Resource>vertexResource_;
    VertexData* vertexData_ = nullptr;

    DirectX::ScratchImage mipImages_;
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_;
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource_;

};
