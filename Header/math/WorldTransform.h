#pragma once

#include"../Header/math/Vector3.h"
#include"../Header/math/Matrix4x4.h"

struct WorldTransform {

    Vector3 scale;
    Vector3 rotate;
    Vector3 transform;
    Matrix4x4 matWorld;
};