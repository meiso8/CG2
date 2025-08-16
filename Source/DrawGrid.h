#pragma once

#include"LineMesh.h"
#include"Cube.h"

class DrawGrid {
public:
    DrawGrid(const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera& camera, ModelConfig& mc);
    void Draw(ShaderResourceView& srv);
private:
    LineMesh line_[102];
    Cube cube_[2];
};
