#include "CollisionManager.h"

// コライダーリストをクリアする関数
void CollisionManager::ClearList() { colliders_.clear(); }

void CollisionManager::CheckCollisionPair(Collider* colliderA, Collider* colliderB) {

	// 衝突フィルタリング
	if (((colliderA->GetAttribute() & colliderB->GetMask()) == 0b0) || ((colliderB->GetAttribute() & colliderA->GetMask()) == 0b0)) {
		return;
	}

	Vector3 posA, posB;

	posA = colliderA->GetWorldPosition();
	posB = colliderB->GetWorldPosition();

	float distance = Length(posA - posB);

	if (distance <= colliderA->GetRadius() + colliderB->GetRadius()) {
		colliderA->OnCollision();
		colliderB->OnCollision();
	}
};

// コライダーを引数で受け取り、コライダーリストに登録する関数
void CollisionManager::RegisterList(Collider* player, const std::list<Mirror*>& mirrors) {

	// コライダーをリストに登録
	colliders_.push_back(player);

	// 敵弾全てについて
	for (Mirror* mirror : mirrors) {
		colliders_.push_back(mirror);
	}
};

void CollisionManager::CheckAllCollisions() {
	// リスト内のベアを総当たり
	std::list<Collider*>::iterator itrA = colliders_.begin();
	for (; itrA != colliders_.end(); ++itrA) {

		// イテレータAからコライダーAを取得する
		Collider* colliderA = *itrA;

		// イテレータBはイテレータAの次の要素から回す（重複判定を回避）
		std::list<Collider*>::iterator itrB = itrA;
		itrB++;

		for (; itrB != colliders_.end(); ++itrB) {
			// イテレータBからコライダーを取得する
			Collider* colliderB = *itrB;

			// ペアの当たり判定	// コライダーAとコライダーBの当たり判定
			CheckCollisionPair(colliderA, colliderB);
		}
	}
};
