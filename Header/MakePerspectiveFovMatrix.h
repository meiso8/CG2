#pragma once

#include"../Header/cotangent.h"
#include"../Header/Matrix4x4.h"

//1.透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
