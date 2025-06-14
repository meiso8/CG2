#pragma once
#include<wrl.h>
#include<d3d12.h>
#include<cstdint>

class Fence
{
public:
    void Create(const Microsoft::WRL::ComPtr <ID3D12Device>& device);
    Microsoft::WRL::ComPtr<ID3D12Fence> GetFence() {
        return fence_
            ;
    };

    uint64_t GetValue() {
        return fenceValue_
            ;
    };
    void AddValue();

    void SendSignal(const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& commandQueue);

    void CheckValue(HANDLE fenceEvent);

private:
    //初期値0でFenceを作る
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    uint64_t fenceValue_ = 0;
};

