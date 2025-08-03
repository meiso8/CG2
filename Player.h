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
    void Draw(Camera& camera);
    Vector3 GetWorldPosition()override;
    void OnCollision() override;
private:
    WorldTransform worldTransform_ = {0.0f};
    Model* model_;
    int hp_ = 100;
};

