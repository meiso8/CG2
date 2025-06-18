#pragma once

#include<cstdint>
#include"../Header/math/WorldTransform.h"
#include"../Header/Camera.h"

class Player
{
public:
    void Initialize(Camera* camera);
    void Update();
    void Draw();
private:
    uint32_t textureHandle_ = 0u;
    WorldTransform worldTransoform_;
    Camera* camera_;
};
