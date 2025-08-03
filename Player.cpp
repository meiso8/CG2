#include "Player.h"
#include"Header/Model.h"
#include"Header/Input.h"


void Player::Init(Model& model) {

    model_ = &model;

    worldTransform_.Initialize();
};

void Player::Update() {

    if (Input::GetInstance()->IsPushKey(DIK_A)) {
        worldTransform_.translate_.x--;
    }

    if (Input::GetInstance()->IsPushKey(DIK_D)) {
        worldTransform_.translate_.x++;
    }

    if (Input::GetInstance()->IsPushKey(DIK_W)) {
        worldTransform_.translate_.y++;
    }

    if (Input::GetInstance()->IsPushKey(DIK_S)) {
        worldTransform_.translate_.y--;
    }

    WorldTransformUpdate(worldTransform_);

};

void Player::Draw(Camera& camera) {
    model_->PreDraw(PSO::TRIANGLE);
    model_->Draw(worldTransform_.matWorld_, camera);

};
