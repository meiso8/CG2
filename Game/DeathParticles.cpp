#include "DeathParticles.h"
#include "../Header/math/MakeRotateMatrix.h"
#include "../Header/math/TransformVector.h"
#include"../Header/Camera.h"
#include"../Header/Model.h"
#include "WorldTransformUpdate.h"
#include <algorithm>


void DeathParticles::Initialize(Model* model, Camera* camera, const Vector3& position) {
	// NULLポインタチェック
	assert(model);

	// 引数として受け取ったデータをメンバ変数に記録する
	this->model_ = model;
	this->camera_ = camera;

	// ワールド変換の初期化
	for (auto& worldTransform : worldTransforms_) {
		worldTransform.Initialize();
		worldTransform.translation_ = position;
	}
};

void DeathParticles::Update() {

	// 早期リターン
	if (isFinished_) {
		return;
	}

	// カウンターを1フレーム分の秒数進める
	counter_ += 1.0f / 60.0f;

	// 存続時間の上限に達したら
	if (counter_ >= kDuration) {
		counter_ = kDuration;
		// 終了扱いにする
		isFinished_ = true;
	}

	for (uint32_t i = 0; i < kNumParticles; ++i) {
		// 基本となるベクトル
		Vector3 velocity = {kSpped, 0, 0};
		// 回転角を計算する
		float angle = kAngleUnit * i;
		// Z軸回り回転行列
		Matrix4x4 matrixRotation = MakeRotateZMatrix(angle);
		// 基本ベクトルを回転させて速度ベクトルを得る
		velocity = TransformVector(velocity, matrixRotation);
		// 移動処理
		worldTransforms_[i].translation_ += velocity;
	}

	// ==============================
	// 行列を定数バッファに転送
	// ==============================
	for (auto& worldTransform : worldTransforms_) {
		WorldTransformUpdate(worldTransform);
	}

	// 色を変更
	color_.w = std::clamp(kDuration - counter_, 0.0f, 1.0f);
	model_->SetColor(color_);

};

void DeathParticles::Draw() {

	// 早期リターン
	if (isFinished_) {
		return;
	}

	// 3Dモデル描画前処理
	model_->PreDraw();

	for (auto& worldTransform : worldTransforms_) {
		// 3Dモデルを描画
		model_->Draw();
	}

};