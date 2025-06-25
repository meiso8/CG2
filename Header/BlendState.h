#pragma once

#include<d3d12.h>

class BlendState {
public:
    void Create();
    D3D12_BLEND_DESC& GetDesc()
    {
        return blendDesc_;
    }
private:
    //書き込む色要素を決めることなども出来る
    D3D12_BLEND_DESC blendDesc_{};

};


