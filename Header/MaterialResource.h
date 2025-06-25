#pragma once

#include"../Header/Material.h"

//ComPtr(コムポインタ)
#include<wrl.h>
#include<d3d12.h>

class MaterialResource {

public:
    void CreateMaterial(const Microsoft::WRL::ComPtr<ID3D12Device>& device, bool lighting);
    Material* GetMaterial() {
        return materialData_;
    };
    Microsoft::WRL::ComPtr <ID3D12Resource>& GetMaterialResource() {
        return materialResource_
            ;
    };

    void SetMaterialData(Material* material) {
        materialData_ = material;
    }

    void SetUV(const Matrix4x4& transform) {
        materialData_->uvTransform = transform
            ;
    }

private:
    Microsoft::WRL::ComPtr <ID3D12Resource> materialResource_ = nullptr;
    Material* materialData_ = nullptr;
};