#include "WorldTransformUpdate.h"
#include "../Header/math/MakeAffineMatrix.h"

void WorldTransformUpdate(WorldTransform& worldTransform) {
	// スケール、回転、平行移動を合成して行列を計算する
	worldTransform.matWorld_ = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotate_, worldTransform.translate_);
	// 定数バッファへの書きこみ
	worldTransform.TransferMatrix();
}