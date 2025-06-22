#pragma once
#include"Matrix4x4.h"
#include"Vector3.h"

//二項演算子 Binary Operator
Vector3 operator+(const Vector3& v1, const Vector3& v2);
Vector3 operator-(const Vector3& v1, const Vector3& v2);
Vector3 operator*(float s, const Vector3& v);
Vector3 operator+(const Vector3& v, float s);
Vector3 operator+(float s, const Vector3& v);
Vector3 operator*(const Vector3& v, float s);
Vector3 operator/(const Vector3& v, float s);

//単項演算子 Unary Operator
Vector3 operator-(const Vector3& v);
Vector3 operator+(const Vector3& v);


Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2);
Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2);
Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2);