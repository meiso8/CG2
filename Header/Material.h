#pragma once
#include"math/Vector4.h"
#include"math/Matrix4x4.h"
#include<cstdint>

struct Material {
    Vector4 color;
    int32_t lightType;
    float padding[3];
    Matrix4x4 uvTransform;
};

