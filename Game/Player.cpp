#define NOMINMAX // std::maxの置き換えが行われないため

#include "Player.h"
#include"../Header/ImGuiClass.h"
#include "../Header/math/AABB.h"
#include "../Header/math/EaseInOutCubic.h"

#include"../Header/Log.h"
#include "MapChipField.h"
#include "WorldTransformUpdate.h"
#include <algorithm>
#include <numbers>
#include<cassert>

void Player::Initialize(Model* model, Camera* camera, Input* input, const Vector3& position) {
    // NULLポインタチェック
    assert(model);

    // 引数として受け取ったデータをメンバ変数に記録する
    this->model_ = model;
    this->camera_ = camera;
    this->input_ = input;

    // ワールド変換の初期化
    worldTransform_.Initialize();
    worldTransform_.translate_ = position;
    worldTransform_.SetRotationY(std::numbers::pi_v<float> / 2.0f);
}

void Player::InputMove() {

    if (input_->IsPushKey(DIK_RIGHT) || input_->IsPushKey(DIK_LEFT)) {

        // 左右加速
        Vector3 acceleration = {};

        if (input_->IsPushKey(DIK_RIGHT)) {
            // 右入力時

            if (velocity_.x < 0.0f) {
                // 左方向に行っていたとき ブレーキをかける
                velocity_.x *= (1.0f - kAttenuation);
            }

            acceleration.x += kAcceleration;

            if (lrDirection_ != LRDirection::kRight) {
                lrDirection_ = LRDirection::kRight;
                // 旋回開始時の角度を記録する
                turnFirstRotationY_ = worldTransform_.GetRotate().y;
                // 旋回タイマーに時間を設定する
                turnTimer_ = 1.0f;
            }

        } else if (input_->IsPushKey(DIK_LEFT)) {
            // 左入力時

            if (velocity_.x > 0.0f) {
                // 右方向に行っていたときブレーキかける
                velocity_.x *= (1.0f - kAttenuation);
            }

            acceleration.x -= kAcceleration;

            if (lrDirection_ != LRDirection::kLeft) {
                lrDirection_ = LRDirection::kLeft;
                // 旋回開始時の角度を記録する
                turnFirstRotationY_ = worldTransform_.GetRotate().y;
                // 旋回タイマーに時間を設定する
                turnTimer_ = 1.0f;
            }
        }

        // 加速or減速
        velocity_ += acceleration;

        // 最大速度制限 std::clampは加減上限の間に収める
        velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

    } else {
        // キーを押していないとき 減衰する
        velocity_.x *= (1.0f - kAttenuation);
    }

    if (input_->IsPushKey(DIK_UP)) {

        // ジャンプ初速
        velocity_ += Vector3(0.0f, kJumpAcceleration, 0.0f);
    }
};

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {

    Vector3 offsetTable[kNumCorner] = {
        {+kWidth / 2.0f, -kHeight / 2.0f, 0.0f}, //  kRightBottom
        {-kWidth / 2.0f, -kHeight / 2.0f, 0.0f}, //  kLeftBottom
        {+kWidth / 2.0f, +kHeight / 2.0f, 0.0f}, //  kRightTop
        {-kWidth / 2.0f, +kHeight / 2.0f, 0.0f}  //  kLeftTop
    };

    return center + offsetTable[static_cast<uint32_t>(corner)];
};

Vector3 Player::UDLRPositon(const Vector3& center, UDLR udlr) {

    Vector3 offsetTable[kNumUDLR] = {
        {0.0f,           +kHeight / 2.0f, 0.0f}, // kTop
        {0.0f,           -kHeight / 2.0f, 0.0f}, // kBottom
        {-kWidth / 2.0f, 0.0f,            0.0f}, // kLeft
        {+kWidth / 2.0f, 0.0f,            0.0f}, //	kRight
    };

    return center + offsetTable[static_cast<uint32_t>(udlr)];
};

void Player::CheckCollisionTop(CollisionMapInfo& info) {

    // 上昇アリ？//上に向かっているとき以外は当たり判定をスキップする
    if (info.moveVol.y <= 0) {
        // 早期リターン　ガード節
        return;
    }

    // 移動後の4つの角の座標
    std::array<Vector3, kNumCorner> positionsNew;

    for (uint32_t i = 0; i < positionsNew.size(); ++i) {
        positionsNew[i] = CornerPosition(worldTransform_.translate_ + info.moveVol, static_cast<Corner>(i));
    }

    MapChipType mapChipType;
    MapChipType mapChipTypeNew;

    // 真上の当たり判定を行う
    bool hit = false;
    // 左上点の判定
    MapChipField::IndexSet indexSet;
    indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
    mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
    mapChipTypeNew = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
    if (mapChipType == MapChipType::kBlock && mapChipTypeNew != MapChipType::kBlock) {
        hit = true;
    }

    // 右上点の判定
    indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
    mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
    mapChipTypeNew = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
    if (mapChipType == MapChipType::kBlock && mapChipTypeNew != MapChipType::kBlock) {
        hit = true;
    }

    if (hit) {
        // めり込みを排除する方向に移動量を設定
        indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translate_ + info.moveVol + Vector3(0, kHeight / 2.0f, 0));
        // めり込み先ブロックの範囲矩形
        MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
        info.moveVol.y = std::max(0.0f, rect.bottom - worldTransform_.translate_.y - kHeight / 2.0f - kBlank);
        // 天井に当たったことを記録する
        info.isCeilingHit = true;
    }
};

void Player::CheckCollisionBottom(CollisionMapInfo& info) {

    // 降下しているか判定
    if (info.moveVol.y >= 0) {
        // 早期リターン　ガード節
        return;
    }

    // 移動後の4つの角の座標
    std::array<Vector3, kNumCorner> positionsNew;

    for (uint32_t i = 0; i < positionsNew.size(); ++i) {
        positionsNew[i] = CornerPosition(worldTransform_.translate_ + info.moveVol, static_cast<Corner>(i));
    }

    MapChipType mapChipType;
    MapChipType mapChipTypeNext;

    // 真下の当たり判定を行う
    bool hit = false;

    // 左下点の判定
    MapChipField::IndexSet indexSet;
    indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
    mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
    mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
    if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
        hit = true;
    }

    // 右下点の判定
    indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
    mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
    mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
    if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
        hit = true;
    }

    if (hit) {
        // 現座標が壁の外か判定

        // UDLRPositon(worldTransform_.translation_ + info.moveVol, UDLR::kBottom);

        // めり込みを排除する方向に移動量を設定
        indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translate_ + info.moveVol + Vector3(0, -kHeight / 2.0f, 0));

        MapChipField::IndexSet indexSetNow;
        indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translate_ + Vector3(0, -kHeight / 2.0f, 0));

        if (indexSetNow.yIndex != indexSet.yIndex) {

            // めり込み先ブロックの範囲矩形
            MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
            info.moveVol.y = std::min(0.0f, rect.top - worldTransform_.translate_.y + kHeight / 2.0f + kBlank);
            // 地面に当たったことを記録する
            info.isLanding = true;
        }
    }
};
void Player::CheckCollisionRight(CollisionMapInfo& info) {

    // 右に移動しているか
    if (info.moveVol.x <= 0.0f) {
        return;
    }

    // 移動後の4つの角の座標
    std::array<Vector3, kNumCorner> positionsNew;

    for (uint32_t i = 0; i < positionsNew.size(); ++i) {
        positionsNew[i] = CornerPosition(worldTransform_.translate_ + info.moveVol, static_cast<Corner>(i));
    }

    MapChipType mapChipType;
    // 真下の当たり判定を行う
    bool hit = false;

    // 右上点の判定
    MapChipField::IndexSet indexSet;
    indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
    mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
    if (mapChipType == MapChipType::kBlock) {
        hit = true;
    }

    // 右下点の判定
    indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
    mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
    if (mapChipType == MapChipType::kBlock) {
        hit = true;
    }

#ifdef _DEBUG
    ImGui::Text("hit : %d", hit);
    ImGui::Text("RightTop : %f", positionsNew[kRightTop]);
    ImGui::Text("RightBottom : %f", positionsNew[kRightBottom]);
#endif

    if (hit) {

        // めり込みを排除する方向に移動量を設定
        indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
        // めり込み先ブロックの範囲矩形
        MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
        info.moveVol.x = std::max(0.0f, rect.left - worldTransform_.translate_.x - kWidth / 2.0f - kBlank);
        // 壁に当たったことを記録する
        info.isWallHit = true;
        ImGui::Text("rect.left %f", rect.left);
        ImGui::Text("index x : %d, y : %d", indexSet.xIndex, indexSet.yIndex);
        ImGui::Text("moveVol.x : %f", info.moveVol.x);
    }
};
void Player::CheckCollisionLeft(CollisionMapInfo& info) {

    if (info.moveVol.x >= 0.0f) {
        return;
    }

    // 移動後の4つの角の座標
    std::array<Vector3, kNumCorner> positionsNew;

    for (uint32_t i = 0; i < positionsNew.size(); ++i) {
        positionsNew[i] = CornerPosition(worldTransform_.translate_ + info.moveVol, static_cast<Corner>(i));
    }

    MapChipType mapChipType;
    // 左の当たり判定を行う
    bool hit = false;

    // 左上点の判定
    MapChipField::IndexSet indexSet;
    indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
    mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
    if (mapChipType == MapChipType::kBlock) {
        hit = true;
    }

    // 左下点の判定
    indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
    mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
    if (mapChipType == MapChipType::kBlock) {
        hit = true;
    }

#ifdef _DEBUG
    ImGui::Text("hit : %d", hit);
    ImGui::Text("RightTop : %f", positionsNew[kLeftTop]);
    ImGui::Text("RightBottom : %f", positionsNew[kLeftBottom]);
#endif

    if (hit) {

        // めり込みを排除する方向に移動量を設定
        indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
        // めり込み先ブロックの範囲矩形
        MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
        info.moveVol.x = std::min(0.0f, rect.right - worldTransform_.translate_.x + kWidth / 2.0f + kBlank);
        // 壁に当たったことを記録する
        info.isWallHit = true;
        ImGui::Text("rect.right %f", rect.right);
        ImGui::Text("index x : %d, y : %d", indexSet.xIndex, indexSet.yIndex);
        ImGui::Text("moveVol.x : %f", info.moveVol.x);
    }
};

void Player::CheckCollisionMap(CollisionMapInfo& info) {

    CheckCollisionTop(info);
    CheckCollisionBottom(info);
    CheckCollisionRight(info);
    CheckCollisionLeft(info);
};

// 判定結果を反映して移動させる
void Player::ApplyResultAndMove(const CollisionMapInfo& info) {
    // 移動
    worldTransform_.translate_ += info.moveVol;
};

void Player::CeilingHit(const CollisionMapInfo& info) {

    if (info.isCeilingHit) {

        Log("hit ceiling\n");
        velocity_.y = 0.0f;
    }
};

void Player::SwitchOnGround(const CollisionMapInfo& info) {

    if (onGround_) {

        // 設置状態の処理

        // ジャンプ開始
        if (velocity_.y > 0.0f) {
            // 空中状態に移行
            onGround_ = false;
        } else {

            // 落下判定　左右移動や床の消滅による落下を考える
            //   移動後の4つの角の座標
            std::array<Vector3, kNumCorner> positionsNew;

            for (uint32_t i = 0; i < positionsNew.size(); ++i) {
                positionsNew[i] = CornerPosition(worldTransform_.translate_ + info.moveVol, static_cast<Corner>(i));
            }

            MapChipType mapChipType;
            // 真下の当たり判定を行う
            bool hit = false;

            // 左上点の判定
            MapChipField::IndexSet indexSet;
            positionsNew[kLeftBottom] + Vector3(0, -kBlank, 0);
            indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
            mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
            if (mapChipType == MapChipType::kBlock) {
                hit = true;
            }

            // 右上点の判定
            positionsNew[kRightBottom] + Vector3(0, -kBlank, 0);
            indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
            mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
            if (mapChipType == MapChipType::kBlock) {
                hit = true;
            }

            // 落下中なら空中状態に切り替え
            if (!hit) {
                // 空中状態に切り替え
                onGround_ = false;
            }
        }

    } else {

        // 空襲状態の処理

        // 着地
        if (info.isLanding) {

            // 接地状態へ移行 落下を止める
            onGround_ = true;
            // 摩擦で横方向速度が減衰
            velocity_.x *= (1.0f - kAttenuationLanding);
            // 下方向速度をリセット
            velocity_.y = 0.0f;
        }
    }

    ImGui::Text("landing : %d", info.isLanding);
}

void Player::WallHit(const CollisionMapInfo& info) {

    // 壁接触による減衰
    if (info.isWallHit) {
        velocity_.x *= (1.0f - kAttenuationWall);
    }
}

Vector3 Player::GetWorldPosition() { return { worldTransform_.translate_.x, worldTransform_.translate_.y, worldTransform_.translate_.z }; }

void Player::Update() {

    // ==============================
    // 2.移動量を加味して衝突判定をする
    // ==============================
    // 衝突情報を初期化
    CollisionMapInfo collisionMapInfo;

    if (behaviorRequest_ != Behavior::kUnKnown) {

        // 振る舞いを変更する
        behavior_ = behaviorRequest_;
        // 各振る舞いごとの初期化を実行
        switch (behavior_) {
        case Behavior::kRoot:
            // 通常行動初期化
            BehaviorRootInitialize();
            break;
        case Behavior::kAttack:
            // 攻撃行動初期化
            BehaviorAttackInitialize();
            break;
        }

        // 振る舞いリクエストをリセット
        behaviorRequest_ = Behavior::kUnKnown;
    }

    switch (behavior_) {
    case Behavior::kRoot:
        BehaviorRootUpdate();
        break;
    case Behavior::kAttack:
        BehaviorAttackUpdate(collisionMapInfo);
        break;
    }

    // 移動量に速度の値をコピー
    collisionMapInfo.moveVol = velocity_;

    // マップ衝突チェック
    CheckCollisionMap(collisionMapInfo);

    // ==============================
    // 3.判定結果を反映して移動させる
    // ==============================

    // 判定結果を反映して移動させる
    ApplyResultAndMove(collisionMapInfo);

    ImGui::Begin("Player");
    ImGui::DragFloat3("scale", &worldTransform_.scale_.x);
    ImGui::DragFloat3("rotate_", &worldTransform_.rotate_.x);
    ImGui::DragFloat3("translation_", &worldTransform_.translate_.x);
    ImGui::DragFloat3("velocity", &velocity_.x);
    ImGui::Text("onGround_ : %d", onGround_);
    ImGui::Text("isWallHit : %d ", collisionMapInfo.isWallHit);
    ImGui::DragFloat3("moveVol", &collisionMapInfo.moveVol.x);

    ImGui::End();

    // ==============================
    // 4.天井に接触している場合の処理
    // ==============================

    CeilingHit(collisionMapInfo);

    // ==============================
    // 5.壁に接触している場合の処理
    // ==============================

    WallHit(collisionMapInfo);

    // ==============================
    // 6.接地状態の切り替え
    // ==============================

    SwitchOnGround(collisionMapInfo);

    // ==============================
    // 7.旋回制御
    // ==============================

    if (turnTimer_ > 0.0f) {

        turnTimer_ -= 1.0f / 60.0f;

        // 左右の自キャラ角度テーブル
        float destinationRotationYTable[] = { std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> *3.0f / 2.0f };

        // 状態に応じた角度の取得
        float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
        // 角度を設定
        worldTransform_.rotate_.y = EaseInOutCubic(destinationRotationY);
    }

    // ==============================
    // 8.行列を定数バッファに転送
    // ==============================

    WorldTransformUpdate(worldTransform_);
}

void Player::Draw() {

    // 3Dモデル描画前処理
    Model::PreDraw(dxCommon->GetCommandList());

    // 3Dモデルを描画
    model_->Draw(worldTransform_, *camera_);

    Model::PostDraw();
}

AABB Player::GetAABB() {

    Vector3 worldPos = GetWorldPosition();

    AABB aabb{};

    aabb.min = { worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f };
    aabb.max = { worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f };

    return aabb;
}

// 衝突応答
void Player::OnCollision(const Enemy* enemy) {

    (void)enemy;

    // デスフラグを立てる
    isDead_ = true;
};

void Player::BehaviorRootUpdate() {

    if (input_->IsPushKey(DIK_SPACE)) {

        // 攻撃ビヘイビアをリクエスト
        behaviorRequest_ = Behavior::kAttack;
    }

    // 接地状態
    if (onGround_) {

        // ==============================
        // 1.移動入力
        // ==============================
        InputMove();

    } else {
        // 空中

        // 落下速度
        velocity_ += Vector3(0.0f, -kGravityAcceleration, 0.0f);
        // 落下速度制限
        velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
    }
}

void Player::BehaviorAttackUpdate(CollisionMapInfo collitionMapInfo) {

    // 攻撃動作用の速度
    Vector3 velocity{};

    // 予備動作
    attackParameter_++;

    switch (attackPhase_) {
    case AttackPhase::kPowerGet:
    default:
        /*float t = static_cast<float>(attackParameter_) / 10;*/
        // worldTransform_.scale_.z = EaseOut(1.0f, 0.3f, t);
        // worldTransform_.scale_.y = EaseOut(1.0f, 1.6f, t);
        if (attackParameter_ >= 1) {

            attackPhase_ = AttackPhase::kMoveStraight;
            attackParameter_ = 0;
        }
        break;
    case AttackPhase::kMoveStraight:
        /*t = static_cast<float>(attackParameter_) / 10;*/
        // worldTransform_.scale_.z = EaseOut(0.3f, 1.3f, t);
        // worldTransform_.scale_.y = EaseIn(1.6f, 0.7f, t);

        if (lrDirection_ == LRDirection::kRight) {

            velocity.x = +attackSpeedX_;
        } else {
            velocity.x = -attackSpeedX_;
        }

        if (attackParameter_ >= 9) {

            attackPhase_ = AttackPhase::kAftertaste;
            attackParameter_ = 0;
        }

        break;
    case AttackPhase::kAftertaste:
        /*t = static_cast<float>(attackParameter_) / 10;*/
        // worldTransform_.scale_.z = EaseOut(1.3f, 1.0f, t);
        // worldTransform_.scale_.y = EaseOut(0.7f, 1.0f, t);

        break;
    }

    // 限定の時間経過で攻撃終了して通常攻撃に戻す
    if (attackParameter_ >= attackTime_) {
        behaviorRequest_ = Behavior::kRoot;
    }

    // 衝突情報を初期化
    collitionMapInfo.moveVol = velocity;
}

// 通常行動初期化
void Player::BehaviorRootInitialize() {

};

// 攻撃行動初期化
void Player::BehaviorAttackInitialize() {
    attackParameter_ = 0;
    velocity_ = { 0 };
};