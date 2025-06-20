
#pragma once


#include "../Header/Model.h"
#include"../Header/Camera.h"

/// @brief 天球
class Skydome {
public:
	/// @brief 初期化
	void Initialize(Model* model,Camera* camera);
	/// @brief 更新
	void Update();
	/// @brief 描画
	void Draw();

private:
	// ワールド変換データ
	WorldTransform worldTransform_;
	// モデル
	Model* model_ = nullptr;
	// カメラ
	Camera* camera_ = nullptr;
};