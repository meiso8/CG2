#include "Player.h"

#include"Header/Input.h"
#include"CollisionConfig.h"
#include"Header/Model.h"

void Player::Init(Model& model) {

    model_ = &model;


    worldTransform_.Initialize();
    SetRadius(0.5f);

    SetAttribute(kCollisionAttributePlayer);
    SetMask(0xffffffff ^ kCollisionAttributePlayer);

};

void Player::InputMove() {


    if (Input::GetInstance()->IsPushKey(DIK_A)) {
        worldTransform_.translate_.x -= speed_;
    }

    if (Input::GetInstance()->IsPushKey(DIK_D)) {
        worldTransform_.translate_.x += speed_;
    }

    if (Input::GetInstance()->IsPushKey(DIK_W)) {
        worldTransform_.translate_.y += speed_;
    }

    if (Input::GetInstance()->IsPushKey(DIK_S)) {
        worldTransform_.translate_.y -= speed_;
    }

};

void Player::Update() {

    InputMove();

    if (isHit_) {

        model_->GetExpansionData().expansion += 1.0f / 10.0f;

        if (model_->GetExpansionData().expansion >= 0.2f) {
            worldTransform_.Initialize();
            model_->GetExpansionData().expansion = 0.0f;
            isHit_ = false;
        }
    }

    WorldTransformUpdate(worldTransform_);

};

void Player::Draw(Camera& camera) {
    model_->PreDraw(PSO::TRIANGLE);
    model_->Draw(worldTransform_.matWorld_, camera);

};

Vector3 Player::GetWorldPosition() {

    Vector3 worldPos;

    worldPos.x = worldTransform_.matWorld_.m[3][0];
    worldPos.y = worldTransform_.matWorld_.m[3][1];
    worldPos.z = worldTransform_.matWorld_.m[3][2];

    return worldPos;
}

void Player::OnCollision() {

    //HPを減らす
    hp_ -= 10;
    isHit_ = true;

};