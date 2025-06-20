#define NOMINMAX // std::maxの置き換えが行われないため

#include "CameraController.h"
#include "../Header/math/Lerp.h"
#include"../Header/math/WorldTransform.h"
#include "Player.h"
#include <algorithm> // std::max, std::min を使用するために必要

void CameraController::Initialize(Camera* camera) {
    camera_ = camera;
    camera_->Initialize(1280, 720, false);
}

void CameraController::Update() {

    const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
    // 追従対象とオフセットと追従対象の速度からカメラの目標座標を計算
    targetPos_ = targetWorldTransform.translation_ + targetOffset_ + target_->GetVelocity() * kVelocityBias;

    // 座標補間によりゆったり追従
    camera_->GetTranslate() = Lerp(camera_->GetTranslate(), targetPos_, kInterpolationRate);

    // 移送範囲制限
    camera_->SetTarnslate();
    camera_->translation_.x = std::max(camera_->GetTranslate().x, targetWorldTransform.translation_.x + margin.left);
    camera_->translation_.x = std::min(camera_->GetTranslate().x, targetWorldTransform.translation_.x + margin.right);
    camera_->translation_.y = std::max(camera_->GetTranslate().y, targetWorldTransform.translation_.y + margin.bottom);
    camera_->translation_.y = std::min(camera_->GetTranslate().y, targetWorldTransform.translation_.y + margin.top);

    camera_->UpdateMatrix();
}

void CameraController::Reset() {

    // 追従対象のワールドトランスフォーム
    const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
    // 追従対象とオフセットからカメラ座標を計算
    camera_->GetTranslate() = targetWorldTransform.translation_ + targetOffset_;
};