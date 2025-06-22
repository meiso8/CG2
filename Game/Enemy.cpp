
#include "Enemy.h"
#include "../Header/math/AABB.h"
#include "WorldTransformUpdate.h"
#include <cmath>
#include <numbers>
#include<cassert>

void Enemy::Initialize(Model* model, Camera* camera, Vector3& position) {
    // NULLポインタチェック
    assert(model);

    model_ = model;
    camera_ = camera;
    worldTransform_.Initialize();
    worldTransform_.SetTranslation(position);// 初期位置をオリジンにしておく
    worldTransform_.SetRotationY(std::numbers::pi_v<float> *3.0f / 2.0f);

    // 速度を設定する
    velocity_ = { -kWalkSpeed, 0, 0 };
    walkTimer_ = 0.0f;
}

void Enemy::Update() {

    // 移動
    worldTransform_.translate_ += velocity_;

    walkTimer_ += 1.0f / 60.0f;
    // 回転アニメーション
    float param = std::sin(std::numbers::pi_v<float> *walkTimer_ / kWalkMotionTime);
    float radian = kWalkMotionStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;
    worldTransform_.SetRotationX(radian * std::numbers::pi_v<float> / 180.0f);

    // ==============================
    // 行列を定数バッファに転送
    // ==============================

    WorldTransformUpdate(worldTransform_);
}

void Enemy::Draw() {

    // 3Dモデル描画前処理
    Model::PreDraw(dxCommon->GetCommandList());

    // 3Dモデルを描画
    model_->Draw(worldTransform_, *camera_);

    Model::PostDraw();
}

AABB Enemy::GetAABB() {

    Vector3 worldPos = GetWorldPosition();

    AABB aabb;

    aabb.min = { worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f };
    aabb.max = { worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f };

    return aabb;
}

Vector3 Enemy::GetWorldPosition() { return { worldTransform_.translate_.x, worldTransform_.translate_.y, worldTransform_.translate_.z }; }

void Enemy::OnCollision(const Player* player) {

    // 無意味な処理を入れることで警告を抑制できる
    (void)player;
};
