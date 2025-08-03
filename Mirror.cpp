#include "Mirror.h"
#include"Header/Model.h"
#include"CollisionConfig.h"


void Mirror::Init(Model& model) {

    model_ = &model;
    worldTransform_.Initialize();
    SetRadius(0.5f);
    SetAttribute(kCollisionAttributeMirror);
    SetMask(0xffffffff ^ kCollisionAttributeMirror);
};

void Mirror::Update() {

    worldTransform_.rotate_.y += 1.0f / 120.0f;
    WorldTransformUpdate(worldTransform_);


    if (isBroken_) {

        model_->GetExpansionData().expansion += 1.0f / 10.0f;

        if (model_->GetExpansionData().expansion >= 1.0f) {
            model_->GetExpansionData().expansion = 1.0f;
            isDead_ = true;
        }
    }
};

void Mirror::Draw(Camera& camera) {
    model_->PreDraw(PSO::TRIANGLE);
    model_->Draw(worldTransform_.matWorld_, camera);

};

Vector3 Mirror::GetWorldPosition() {

    Vector3 worldPos;

    worldPos.x = worldTransform_.matWorld_.m[3][0];
    worldPos.y = worldTransform_.matWorld_.m[3][1];
    worldPos.z = worldTransform_.matWorld_.m[3][2];

    return worldPos;
}


void Mirror::OnCollision() {
    isBroken_ = true;

};


Mirror::~Mirror() {
    delete model_;
}