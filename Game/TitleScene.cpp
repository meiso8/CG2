#include "TitleScene.h"
#include "WorldTransformUpdate.h"


void TitleScene::Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera& camera,
    CommandList& commandList,
    D3D12_VIEWPORT& viewport, D3D12_RECT& scissorRect,
    const Microsoft::WRL::ComPtr<ID3D12RootSignature>& rootSignature, PSO& pso,
    const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap) {

    // カメラの初期化
    camera_.Initialize(1280, 720, false);
    camera_.SetFarZ(1000.0f);
    camera_.SetTarnslateZ(-5.0f);
    camera_.Update();

    // プレイヤーの生成 OBJからの生
    playerModel_ = new Model(camera_, commandList_, viewport, scissorRect, rootSignature, pso, directionalLightResource);
    playerModel_->Create("resources/player", "player.obj", device, srvDescriptorHeap);

    // タイトルテキストモデルの生成
    textModel_ = new Model(camera_, commandList_, viewport, scissorRect, rootSignature, pso, directionalLightResource);
    textModel_->Create("resources/title", "title.obj", device, srvDescriptorHeap);

    worldTransform_.Initialize();

    fade_ = new Fade();
    fade_->Initialize(device, camera, commandList, viewport, scissorRect, rootSignature, pso, srvDescriptorHeap);
    fade_->Start(Fade::Status::FadeIn, 1.0f);
};
void TitleScene::Update() {

    WorldTransformUpdate(worldTransform_);

    switch (phase_) {
    case Phase::kFadeIn:

        fade_->Update();

        if (fade_->IsFinished()) {
            phase_ = Phase::kMain;
        }

        break;
    case Phase::kMain:

        if (input_.IsPushKey(DIK_SPACE)) {
            fade_->Start(Fade::Status::FadeOut, 1.0f);
            phase_ = Phase::kFadeOut;
        }

        break;
    case Phase::kFadeOut:

        fade_->Update();

        if (fade_->IsFinished()) {
            finished_ = true;
        }

        break;
    }
};

void TitleScene::Draw() {


    // 3Dモデル描画前処理
    Model::PreDraw();

    // 3Dモデルを描画
    textModel_->Draw(worldTransform_, camera_);
    playerModel_->Draw(worldTransform_, camera_);

    Model::PostDraw();

    switch (phase_) {
    case Phase::kFadeIn:
        fade_->Draw(*dxCommon->GetCommandList());
        break;
    case Phase::kMain:
        break;
    case Phase::kFadeOut:

        fade_->Draw(*dxCommon->GetCommandList());
        break;
    }
};

TitleScene::~TitleScene() {

    if (playerModel_) {
        delete playerModel_;
    }
    if (textModel_) {
        delete textModel_;
    }

    delete fade_;
}