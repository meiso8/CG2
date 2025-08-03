#pragma once

#include "Collider.h"
#include "Header/math/Length.h"

#include <list>

#include "Mirror.h"

class CollisionManager {
public:
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);
	void RegisterList(Collider* player, const std::list<Mirror*>& mirrors);
	void ClearList();
	void CheckAllCollisions();

private:
	// コライダーリスト
	std::list<Collider*> colliders_;
};
