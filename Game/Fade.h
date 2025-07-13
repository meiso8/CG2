#pragma once
#include"../Header/Sprite.h"

/// @brief フェード
class Fade {
public:
    // フェードの状態
    enum class Status {
        None,    // フェードなし
        FadeIn,  // フェードイン中
        FadeOut, // フェードアウト中
    };

    void Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera& camera,
        CommandList& commandList,
        D3D12_VIEWPORT& viewport, D3D12_RECT& scissorRect,
        const Microsoft::WRL::ComPtr<ID3D12RootSignature>& rootSignature, PSO& pso,
        const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap);
    void Update();
    void Draw(const Microsoft::WRL::ComPtr <ID3D12Resource>& directionalLightResource,
        const Microsoft::WRL::ComPtr <ID3D12Resource> waveResource,
        const Microsoft::WRL::ComPtr <ID3D12Resource> expansionResource);
    ~Fade();
    void Start(Status status, float duration);
    void Stop();
    //フェード終了判定
    bool IsFinished() const;
private:
    // 現在のフェードの状態
    Status status_ = Status::None;

    // フェードの持続時間
    float duration_ = 0.0f;
    // 経過時間カウンター
    float counter_ = 0.0f;

    // テクスチャハンドル
    uint32_t textureHandle_ = 0;
    Sprite* sprite_ = nullptr;
    ShaderResourceView srv_ = {};
};