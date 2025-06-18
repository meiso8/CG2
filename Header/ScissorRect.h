#pragma once

#include<d3d12.h>
#include<cstdint>

class ScissorRect
{
public:
    D3D12_RECT Create(const int32_t& width, const int32_t& height);

};

