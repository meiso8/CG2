#pragma once

#include "Header/math/WorldTransform.h"

class Model;
class Camera;

class Player
{

public:
    void Init(Model& model);
    void Update();
    void Draw(Camera& camera);
private:
    WorldTransform worldTransform_;
    Model* model_;
};

