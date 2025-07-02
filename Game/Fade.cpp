#include "Fade.h"
#include <algorithm>

const int winWidth = 1280;
const int winHeight = 720;

#include"../Header/Texture.h"

/// @brief フェード
void Fade::Initialize(
    const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera& camera,
    CommandList& commandList,
    D3D12_VIEWPORT& viewport, D3D12_RECT& scissorRect,
    const Microsoft::WRL::ComPtr<ID3D12RootSignature>& rootSignature, PSO& pso,
    const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap) {
    // ファイル名を指定してテクスチャを読み込む

    Texture texture = Texture(device, commandList);
    texture.Load("resources/black.jpg");

    //ShaderResourceViewを作る

    srv_.Create(texture.GetMetadata(), texture.GetTextureResource(), 1, device, srvDescriptorHeap);

    sprite_->Create(device, camera, commandList, viewport, scissorRect, rootSignature, pso);
    sprite_->SetSize(Vector2(winWidth, winHeight));
    sprite_->SetColor(Vector4(0, 0, 0, 0.5f));
};

void Fade::Update() {

    // フェード状態による分岐
    switch (status_) {
    case Status::None:
        // 何もしない

        break;
    case Status::FadeIn:
        // フェードイン中の更新処

        counter_ += 1.0f / 60.0f;
        // フェード継続時間に達したら打ち止め
        if (counter_ >= duration_) {
            counter_ = duration_;
        }
        // 0.0fから1.0fの間で、継続時間がフェード継続時間に近づくほどα値を小さくする
        sprite_->SetColor(Vector4(0, 0, 0, std::clamp(1.0f - counter_ / duration_, 0.0f, 1.0f)));

        break;
    case Status::FadeOut:
        // フェードアウト中の更新処理
        counter_ += 1.0f / 60.0f;
        // フェード継続時間に達したら打ち止め
        if (counter_ >= duration_) {
            counter_ = duration_;
        }
        // 0.0fから1.0fの間で、継続時間がフェード継続時間に近づくほどα値を大きくする
        sprite_->SetColor(Vector4(0, 0, 0, std::clamp(counter_ / duration_, 0.0f, 1.0f)));

        break;
    }
};

void Fade::Draw(const Microsoft::WRL::ComPtr <ID3D12Resource>& directionalLightResource,
    const Microsoft::WRL::ComPtr <ID3D12Resource> waveResource,
    const Microsoft::WRL::ComPtr <ID3D12Resource> expansionResource) {

    if (status_ == Status::None) {
        return;
    }

    sprite_->PreDraw();
    sprite_->Draw(srv_, directionalLightResource, waveResource, expansionResource);

};

void Fade::Start(Status status, float duration) {
    status_ = status;
    duration_ = duration;
    counter_ = 0.0f;
};

void Fade::Stop() { status_ = Status::None; }

bool Fade::IsFinished() const {

    return (counter_ >= duration_) ? true : false;

}

Fade::~Fade() { delete sprite_; }