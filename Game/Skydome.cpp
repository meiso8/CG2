#include "Skydome.h"


#include "3d/WorldTransform.h"


void Skydome::Initialize(Model* model, Camera* camera) {

	//// NULLポインタチェック
	//assert(model);

	// 引数として受け取ったデータをメンバ変数に記録する
	model_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
}

void Skydome::Update() {

	// 行列を定数バッファに転送
	worldTransform_.TransferMatrix();
	// 中身は定数バッファへの書き込み。
}

void Skydome::Draw() {
	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 3Dモデル描画前処理
	Model::PreDraw(dxCommon->GetCommandList());

	// 3Dモデルを描画
	model_->Draw(worldTransform_, *camera_);

	Model::PostDraw();
}