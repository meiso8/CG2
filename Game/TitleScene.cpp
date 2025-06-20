#include "TitleScene.h"
#include "WorldTransformUpdate.h"


void TitleScene::Initialize() {

	// カメラの初期化
	camera_.Initialize();
	camera_.farZ = 1000;
	camera_.translation_.z = -5.0f;
	camera_.UpdateMatrix();

	// プレイヤーの生成 OBJからの生成
	playerModel_ = Model::CreateFromOBJ("player", true);
	// タイトルテキストモデルの生成
	textModel_ = Model::CreateFromOBJ("title", true);

	worldTransform_.Initialize();

	fade_ = new Fade();
	fade_->Initialize();
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

		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
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

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 3Dモデル描画前処理
	Model::PreDraw(dxCommon->GetCommandList());

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