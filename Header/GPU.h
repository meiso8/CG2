#pragma once

#include<wrl.h>
#include<d3d12.h>
#include<dxgi1_6.h>

class GPU
{
public:
    void SettingGPU(const Microsoft::WRL::ComPtr<IDXGIFactory7>& dxgiFactory);
    Microsoft::WRL::ComPtr<ID3D12Device> CreateD3D12Device();
private:
    Microsoft::WRL::ComPtr <IDXGIAdapter4> useAdapter_ = nullptr;

};

