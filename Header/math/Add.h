#pragma once
#include"Vector3.h"
#include"Matrix4x4.h"

//加算
Vector3 Add(const Vector3& v1, const Vector3& v2);

//行列の加法
Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);