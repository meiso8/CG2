#include"../Header/Camera.h"
#include"../Header/Inverse.h"
#include"../Header/MakeAffineMatrix.h"
#include"../Header/Multiply.h"
#include"../Header/MakePerspectiveFovMatrix.h"
#include"../Header/MakeOrthographicMatrix.h"

void Camera::Initialize(const float& width, const float& height, const bool& isOrthographic) {

    viewMatrix_ = Inverse(MakeAffineMatrix(scale_, rotation_, translate_));

    width_ = width;
    height_ = height;

    isOrthographic_ = isOrthographic;

    if (isOrthographic_) {
        //平行投影
        projectionMatrix_ = MakeOrthographicMatrix(0.0f, 0.0f, width_, height_, 0.0f, 100.0f);

    } else {
        //投資投影
        projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, width_ / height_, 0.1f, 100.0f);
    }
}

void Camera::Update() {

    viewMatrix_ = Inverse(MakeAffineMatrix(scale_, rotation_, translate_));

    if (isOrthographic_) {
        //平行投影
        projectionMatrix_ = MakeOrthographicMatrix(0.0f, 0.0f, width_, height_, 0.0f, 100.0f);

    } else {
        //投資投影
        projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, width_ / height_, 0.1f, 100.0f);
    }

}


Matrix4x4 Camera::GetViewProjectionMatrix() {
    return Multiply(viewMatrix_, projectionMatrix_);
}
