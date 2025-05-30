#include "../Header/DebugCamera.h"
#include"../Header/Inverse.h"
#include"../Header/MakeAffineMatrix.h"
#include"../Header/MakeRotateMatrix.h"
#include"../Header/MakePerspectiveFovMatrix.h"
#include"../Header/MakeOrthographicMatrix.h"
#include"../Header/CoordinateTransform.h"
#include"../Header/Multiply.h"
#include<numbers>
#include<cmath>

#define FPS 60

void DebugCamera::Initialize(Input* input, const float& width, const float& height) {

    input_ = input;

    width_ = width;
    height_ = height;

    rotateSpeed_ = std::numbers::pi_v<float> / 10.0f / FPS;
    speed_ = 1.0f;

    viewMatrix_ = Inverse(MakeAffineMatrix({ 1.0f,1.0f,1.0f }, rotation_, translation_));
    projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, width_ / height_, 0.1f, 100.0f);

};

void DebugCamera::Update() {

    InputRotate();
    InputTranslate();

    viewMatrix_ = Inverse(MakeAffineMatrix({ 1.0f,1.0f,1.0f }, rotation_, translation_));

    if (isOrthographic_) {
        //平行投影
        projectionMatrix_ = MakeOrthographicMatrix(0.0f, 0.0f, width_, height_, 0.0f, 100.0f);

    } else {
        //投資投影
        projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, width_ / height_, 0.1f, 100.0f);
    }
}

void DebugCamera::InputTranslate() {


    if (input_->IsPressStateKey(DIK_A)) {
        MoveX(-speed_);
    }

    if (input_->IsPressStateKey(DIK_D)) {
        MoveX(speed_);
    }

    if (input_->IsPressStateKey(DIK_W)) {
        MoveY(speed_);
    }

    if (input_->IsPressStateKey(DIK_S)) {
        MoveY(-speed_);
    }

    if (input_->IsPressStateKey(DIK_Q)) {
        MoveZ(-speed_);
    }

    if (input_->IsPressStateKey(DIK_E)) {
        MoveZ(speed_);
    }
};

void DebugCamera::InputRotate() {

    if (input_->IsPressStateKey(DIK_R)) {

        if (input_->IsTriggerKey(DIK_UP)) {
            rotateSpeed_ *= -1.0f;
        }

        if (input_->IsPressStateKey(DIK_X)) {
            rotation_.x += rotateSpeed_;
        }

        if (input_->IsPressStateKey(DIK_Y)) {
            rotation_.y += rotateSpeed_;
        }

        if (input_->IsPressStateKey(DIK_Z)) {
            rotation_.z += rotateSpeed_;
        }

    }

};

void DebugCamera::MoveZ(const float& speed) {
    //カメラ移動ベクトル
    Vector3 move = { 0.0f,0.0f,speed };
    translation_ += CoordinateTransform(move, MakeRotateXYZMatrix(rotation_));
}

void DebugCamera::MoveX(const float& speed) {
    translation_ += CoordinateTransform({ speed, 0.0f, 0.0f }, MakeRotateXYZMatrix(rotation_));;
};

void DebugCamera::MoveY(const float& speed) {
    translation_ += CoordinateTransform({ 0.0f, speed, 0.0f }, MakeRotateXYZMatrix(rotation_));;
};

Matrix4x4 DebugCamera::GetViewProjectionMatrix() {
    return Multiply(viewMatrix_, projectionMatrix_);
};
