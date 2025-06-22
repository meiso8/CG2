#pragma once
#include"Vector3.h"
#include "Matrix4x4.h"

//スカラー倍
Vector3 Multiply(const float scalar, const Vector3& v);

//行列の積
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);