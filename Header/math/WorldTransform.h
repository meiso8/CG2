#pragma once

#include"Vector3.h"
#include"Matrix4x4.h"

class WorldTransform {
public://一旦public
    Vector3 scale_;
    Vector3 rotate_;
    Vector3 translate_;
    Matrix4x4 matWorld_;
public:
    void Initialize();
    void SetTranslation(const Vector3& translation) {
        translate_ = translation
            ;
    }
    void SetRotationX(float rotateX) {
        rotate_.x = rotateX
            ;
    }

    void SetRotationY(float rotateY) {
        rotate_.y = rotateY
            ;
    }

    Vector3 GetRotate() {
        return rotate_;
    };

};