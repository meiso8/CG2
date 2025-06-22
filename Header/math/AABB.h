#pragma once

#include"../../Header/math/Vector3.h"

// 後でファイル分け
struct AABB {
	Vector3 min;
	Vector3 max;
};

//AABBとAABBの衝突判定
bool IsCollision(const AABB &a, const AABB&b);
