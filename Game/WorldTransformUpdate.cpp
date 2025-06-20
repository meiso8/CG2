#include "WorldTransformUpdate.h"
#include "../Header/math/MakeAffineMatrix.h"

void WorldTransformUpdate(WorldTransform& worldTransform) {
	// スケール、回転、平行移動を合成して行列を計算する
	worldTransform.matWorld_ = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_);
	// 定数バッファへの書きこみ
	worldTransform.TransferMatrix();
}