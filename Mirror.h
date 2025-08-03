#pragma once

#include "Header/math/WorldTransform.h"
#include"Collider.h"
//#include"Header/Model.h"
class Camera;

class Model;

class Mirror :public Collider
{

public:

    void Init(Model& model);
    void Update();
    void Draw(Camera& camera);
    Vector3 GetWorldPosition()override;
    void OnCollision()override;
    void SetTranslate(const Vector3& translate) {
        worldTransform_.translate_ = translate
            ;
    }
    bool& IsBroken() { return isBroken_; }
    bool& IsDead() {
        return isDead_;
    };
    ~Mirror();
private:
    WorldTransform worldTransform_;
    Model* model_;
    bool isBroken_ = false;
    bool isDead_ = false;
};
