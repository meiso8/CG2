#include"../Header/MaterialResource.h"
#include"../Header/math/MakeIdentity4x4.h"
#include"../Header/CreateBufferResource.h"

void MaterialResource::CreateMaterial(const Microsoft::WRL::ComPtr<ID3D12Device>& device, bool lighting) {

    //マテリアル用のリソースを作る。
    materialResource_ = CreateBufferResource(device, sizeof(Material));
    //マテリアルにデータを書き込む

    //書き込むためのアドレスを取得
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData_->enableLighting = lighting;
    materialData_->uvTransform = MakeIdentity4x4();
};

void MaterialResource::SetColor(const Vector4& color) {
    materialData_->color = color;
}