#pragma once
#include "Header/math/Vector3.h"
#include<cstdint>

class Collider {

private:
	float radius_ = 1.0f;
	// 衝突属性
	uint32_t collisionAttribute_ = 0xffffffff;
	// 衝突マスク
	uint32_t CollisionMask_ = 0xffffffff;

public:
	float GetRadius() { return radius_; };
	void SetRadius(float radius) { radius_ = radius; };
	virtual void OnCollision();
	// 純粋仮想関数を持つクラスはインスタンス化できず、
	//  そのクラスを基底クラスとして派生クラスで
	//  具体的な実装を提供することを期待します。
	virtual Vector3 GetWorldPosition() = 0;

	// 衝突属性(自分)を取得
	uint32_t& GetAttribute() { return collisionAttribute_; }
	// 衝突属性(自分)を設定
	void SetAttribute(const uint32_t& attribute) { collisionAttribute_ = attribute; }
	// 衝突マスク(相手)を取得
	uint32_t& GetMask() { return CollisionMask_; }
	// 衝突マスク(相手)を設定
	void SetMask(const uint32_t& mask) { CollisionMask_ = mask; }
};
