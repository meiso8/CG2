#pragma once

#include "Header/math/WorldTransform.h"
#include"Collider.h"

class Model;
class Camera;

class Player :public Collider

{

public:
    void Init(Model& model);
    void Update();
    void InputMove();
    void Draw(Camera& camera);
    Vector3 GetWorldPosition()override;
    void OnCollision() override;
    bool& IsHit() {
        return isHit_;
    };

private:
    WorldTransform worldTransform_ = { 0.0f };
    Model* model_;
    int hp_ = 100;
    bool isHit_ = false;
    float speed_ = 0.5f;

};

