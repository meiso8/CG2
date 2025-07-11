#pragma once

#include"Line.h"

class DrawGrid {
public:
    DrawGrid(const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera& camera, ModelConfig& mc);
    void Draw(ShaderResourceView& srv);
private:
    Line line_[102];

};
