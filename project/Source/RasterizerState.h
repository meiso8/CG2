#pragma once

#include<d3d12.h>

class RasterizerState {
public:
    void Create(D3D12_CULL_MODE cullMode, D3D12_FILL_MODE fillMode);
    D3D12_RASTERIZER_DESC& GetDesc()
    {
        return rasterizerDesc_;
    };
private:
    D3D12_RASTERIZER_DESC rasterizerDesc_{};
};