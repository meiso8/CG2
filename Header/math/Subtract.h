#pragma once

#include"Vector3.h"
#include"Matrix4x4.h"

//ベクトルの減算
Vector3 Subtract(const Vector3& v1, const Vector3& v2);

//行列の減算
Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);