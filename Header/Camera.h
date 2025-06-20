#pragma once
#include"math/Matrix4x4.h"
#include"math/Transform.h"

class Camera {
private:
    Vector3 scale_ = { 1.0f,1.0f,1.0f };
    //XYZ軸回りのローカル回転角
    Vector3 rotation_ = { 0.0f,0.0f,0.0f };
    //ローカル座標
    Vector3 translate_ = { 0.0f,0.0f,0.0f };
    //ビュー行列
    Matrix4x4 viewMatrix_ = {};
    //射影行列
    Matrix4x4 projectionMatrix_ = {};
    float farZ_;

    float width_{};
    float height_{};

    bool isOrthographic_ = false;

public:
    /// @brief 初期化
    void Initialize(const float& width, const float& height, const bool& isOrthographic);
    /// @brief 更新
    void Update();
    void SetViewMatrix(const Matrix4x4& matrix) { viewMatrix_ = matrix; };
    void SetprojectionMatrix(const Matrix4x4& matrix) { projectionMatrix_ = matrix; };
    Matrix4x4 GetViewProjectionMatrix();
    void SetTransform(const Transform& transform) {
        scale_ = transform.scale;
        rotation_ = transform.rotate;
        translate_ = transform.translate;
    };

    Vector3 GetTranslate() {
        return translate_
            ;
    };

    void SetFarZ(const float& farZ) { farZ_ = farZ; };
    void SetTarnslate(const Vector3& translate) { translate_ = translate; };
    void SetTarnslateX(const float& translateX) { translate_.x = translateX; };
    void SetTarnslateY(const float& translateY) { translate_.y = translateY; };
};