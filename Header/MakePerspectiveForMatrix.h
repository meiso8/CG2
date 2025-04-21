#pragma once

#include <cmath>
#include"../Matrix4x4.h"

float cot(float rotate) {
    return std::cos(rotate) / std::sin(rotate);
}

//1.透視投影行列
Matrix4x4 MakePerspectiveForMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
