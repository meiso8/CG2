#pragma once

#include "Fade.h"
#include <cmath>
#include <numbers>
#include"../Header/Model.h"
#include"../Header/Camera.h"
#include"../Header/Input.h"

/// @brief タイトルシーン
class TitleScene {
public:
    enum class Phase {
        kFadeIn,  // フェードイン
        kMain,    // メイン部
        kFadeOut, // フェードアウト
    };

    void Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera& camera,
        CommandList& commandList,
        D3D12_VIEWPORT& viewport, D3D12_RECT& scissorRect,
        const Microsoft::WRL::ComPtr<ID3D12RootSignature>& rootSignature, PSO& pso,
        const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap);
    void Update();
    void Draw();
    // デストラクタのゲッター
    bool IsFinished() const { return finished_; }
    ~TitleScene();

private:
    // テキストモデル
    Model* textModel_;
    // 自キャラモデル
    Model* playerModel_ = nullptr;
    // ワールドトランスフォーム
    WorldTransform worldTransform_;
    // カメラ
    Camera camera_;
    Input input_;

    // カメラ回転速度
    static inline float kCameraRotateSpeedZ = std::numbers::pi_v<float> / 120.0f;
    // 終了フラグ
    bool finished_ = false;
    Fade* fade_ = nullptr;
    // 現在のフェーズ
    Phase phase_ = Phase::kFadeIn;
};