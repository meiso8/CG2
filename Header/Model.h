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
    // Explicitly define a default constructor to resolve the error
    Model() : camera_(nullptr), wvpDate_(nullptr) {}

    void Create(const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera& camera);
    void CreateWorldVP(const Microsoft::WRL::ComPtr<ID3D12Device>& device);

    void Update();

    void InitTraslate();

    void Draw(
        CommandList& commandList,
        D3D12_VERTEX_BUFFER_VIEW& vertexBufferView,
        ShaderResourceView(&srv)[2], const bool& uvCheck
    );

    void DrawCall(CommandList& commandList, ModelData& modelData);

    Material* Getmaterial() { return materialResource_.GetMaterial(); };

    Transform& GetTransformRef() {
        return transform_;
    };

private:
    MaterialResource materialResource_;

    Camera* camera_;

    Microsoft::WRL::ComPtr <ID3D12Resource> wvpResource_;
    TransformationMatrix* wvpDate_;

    //三角形の座標
    Transform transform_ = { 0.0f };
    //三角形の行列
    Matrix4x4 worldMatrix_ = { 0.0f };
    //WVpMatrixを作る
    Matrix4x4 worldViewProjectionMatrix_ = { 0.0f };
};
