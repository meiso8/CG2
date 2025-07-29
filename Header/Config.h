#pragma once
#include"../Header/CommandList.h"
#include"../Header/PSO.h"
#include"../Header/RootSignature.h"

struct ModelConfig {
    CommandList* commandList;
    D3D12_VIEWPORT* viewport;
    D3D12_RECT* scissorRect;
    RootSignature* rootSignature;
    PSO* pso;
    Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
};
