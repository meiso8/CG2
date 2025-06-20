#pragma once

#include"Vector3.h"
#include"Matrix4x4.h"

class WorldTransform {
private:
    Vector3 scale_;
    Vector3 rotate_;
    Vector3 transform_;
    Matrix4x4 matWorld_;
public:
    void Initialize();
    void SetTransform(const Vector3& transform) {
        transform_ = transform
            ;
    };


};