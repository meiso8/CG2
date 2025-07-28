#pragma once  

#include"../Header/CommandList.h"  
#include"../Header/ModelData.h"  
#include"../Header/PSO.h"  
#include"../Header/ShaderResourceView.h"  
#include"../Header/Camera.h"  
#include"../Header/math/Transform.h"  
#include"../Header/TransformationMatrix.h"  
#include"../Header/MaterialResource.h"  
#include"../Header/math/Vector2.h"  
#include"../Header/RootSignature.h"  
#include"../Header/Config.h"
#include"../Header/Balloon.h"
#include"../Header/Wave.h"

class Sprite
{
public:
    void Create(
        const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera& camera, ModelConfig& mc);

    void Update();
    void UpdateUV();

    void PreDraw();
    void Draw(
        ShaderResourceView& srv
    );

    void SetSize(const Vector2& size);
    void SetColor(const Vector4& color);
    void SetTranslate(const Vector3& translate) { transform_.translate; }

    Vector2& GetSize() { return size_; }

    Vector3& GetScaleRef() { return transform_.scale; };
    Vector3& GetRotateRef() { return transform_.rotate; };
    Vector3& GetTranslateRef() { return transform_.translate; };

    Material* GetMaterial() { return materialResource_.GetMaterial(); };

    Vector3& GetUVScaleRef() { return uvTransform_.scale; };
    Vector3& GetUVRotateRef() { return uvTransform_.rotate; };
    Vector3& GetUVTranslateRef() { return uvTransform_.translate; };

private:
    void CreateVertex(const Microsoft::WRL::ComPtr<ID3D12Device>& device);
    void CreateIndexResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device);
    void CreateTransformationMatrix(const Microsoft::WRL::ComPtr<ID3D12Device>& device);
    void CreateMaterial(const Microsoft::WRL::ComPtr<ID3D12Device>& device);
private:
    Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource_{};
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    VertexData* vertexData_ = nullptr;
    D3D12_INDEX_BUFFER_VIEW  indexBufferView_{};
    Microsoft::WRL::ComPtr <ID3D12Resource> indexResource_{};
    uint32_t* indexData_ = nullptr;

    Microsoft::WRL::ComPtr <ID3D12Resource> transformationMatrixResource_ = nullptr;

    Transform transform_{};
    Matrix4x4 worldMatrix_{};
    Matrix4x4 worldViewProjectionMatrix_{};
    TransformationMatrix* transformationMatrixData_ = nullptr;

    Transform uvTransform_ = { 0.0f };
    Matrix4x4 uvTransformMatrix_{};

    MaterialResource materialResource_{};

    Microsoft::WRL::ComPtr<ID3D12Resource> expansionResource_;
    Balloon* expansionData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> waveResource_;
    Wave* waveData = nullptr;

    ModelConfig modelConfig_{};

    Camera* camera_ = nullptr;

    Vector2 size_;
};