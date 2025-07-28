#pragma once


#include"../Header/Texture.h"
#include"../Header/Camera.h"
#include"../Header/ModelData.h"
#include"../Header/MaterialResource.h"
#include"../Header/TransformationMatrix.h"
#include"../Header/Config.h"
#include"../Header/Balloon.h"
#include"../Header/Wave.h"

class Sphere
{
public:

    Sphere(ModelConfig mc)
        : modelConfig_(mc)
    {
    }

    void Create(
        const Microsoft::WRL::ComPtr<ID3D12Device>& device,
        const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap);

    void PreDraw();
    void Draw(
        const Matrix4x4& worldMatrix, Camera& camera, ShaderResourceView& srv);

    Material* GetMaterial() { return materialResource_.GetMaterial(); }

    VertexData* GetVertexData() { return vertexData_; }
    Transform& GetUVTransform() { return uvTransform_; }

    void SetColor(const Vector4& color);
    void UpdateUV();

    ~Sphere();

private:
    void CreateVertex(const Microsoft::WRL::ComPtr<ID3D12Device>& device);
    void CreateWorldVPResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device);
    void CreateMaterial(const Microsoft::WRL::ComPtr<ID3D12Device>& device);
private:
    ShaderResourceView srv_;

    Camera* camera_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
    TransformationMatrix* wvpDate_ = nullptr;

    ModelConfig modelConfig_;

    Matrix4x4 worldViewProjectionMatrix_ = { 0.0f };
    MaterialResource materialResource_;

    Microsoft::WRL::ComPtr<ID3D12Resource> expansionResource_;
    Balloon* expansionData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> waveResource_;
    Wave* waveData_ = nullptr;


    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    VertexData* vertexData_ = nullptr;

    Transform uvTransform_ = { 0.0f };
    Matrix4x4 uvTransformMatrix_{};

    Texture* texture_ = nullptr;

    const uint32_t kSubdivision_ = 16;//分割数

};


