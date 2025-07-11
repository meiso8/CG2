#pragma once

#include"Header/VertexData.h"
#include<wrl.h>
#include<d3d12.h>
#include"Header/Config.h"

#include"Header/CommandList.h"  
#include"Header/ModelData.h"  
#include"Header/PSO.h"  
#include"Header/ShaderResourceView.h"  
#include"Header/Camera.h"  
#include"Header/math/Transform.h"  
#include"Header/TransformationMatrix.h"  
#include"Header/MaterialResource.h"  
#include"Header/math/Vector2.h"  
#include"Header/RootSignature.h"  

class Line
{
public:
    void Create(
        const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera& camera, ModelConfig& mc);

    void Update();

    void PreDraw();
    void Draw(
        ShaderResourceView& srv
    );

    void SetColor(const Vector4& color);
    void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
    void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
    void SetScale(const Vector3& scale) { transform_.scale = scale; }

    Vector3& GetScaleRef() { return transform_.scale; };
    Vector3& GetRotateRef() { return transform_.rotate; };
    Vector3& GetTranslateRef() { return transform_.translate; };
    VertexData& GetVertexData(const uint32_t& index) {
        return vertexData_[index];
    }
    Material* GetMaterial() { return materialResource_.GetMaterial(); };
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

    MaterialResource materialResource_{};

    ModelConfig modelConfig_{};

    Camera* camera_ = nullptr;

};

