#pragma once

#include "../Header/Matrix4x4.h"
#include"../Header/Vector3.h"

//座標変換関数
Vector3 CoordinateTransform(const Vector3& vector, const Matrix4x4& matrix);
