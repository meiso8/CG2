#pragma once


class MapChipField;

struct AABB;

class Enemy;

#include"../Header/math/Vector3.h"
#include "../Header/Model.h"
#include"../Header/Camera.h"

/// @brief 自キャラ
class Player {

public:
	enum class LRDirection { kRight, kLeft };

	struct CollisionMapInfo {
		bool isCeilingHit = false; // 天井
		bool isLanding = false;    // 着地
		bool isWallHit = false;    // 壁
		Vector3 moveVol;
	};

	// 角
	enum Corner {
		kRightBottom,
		kLeftBottom,
		kRightTop,
		kLeftTop,
		kNumCorner // 要素
	};

	enum UDLR {
		kTop,
		kBottom,
		kLeft,
		kRight,
		kNumUDLR,
	};

	// 振る舞い
	enum class Behavior {
		kUnKnown,
		kRoot,  // 通常状態
		kAttack // 攻撃中
	};

	// 攻撃フェーズ(型)
	enum class AttackPhase {
		kPowerGet,     // 溜め
		kMoveStraight, // 直進
		kAftertaste,   // 余韻
	};

public:
	/// @brief 初期化
	/// @param model モデル
	/// @param textureHandle テクスチャハンドル
	void Initialize(Model* model, Camera* camera, const Vector3& position);

	/// @brief 更新
	void Update();

	/// @brief 移動入力
	void InputMove();

	/// @brief マップ衝突判定
	/// @param info 衝突情報
	void CheckCollisionMap(CollisionMapInfo& info);

	void CheckCollisionTop(CollisionMapInfo& info);
	void CheckCollisionBottom(CollisionMapInfo& info);
	void CheckCollisionRight(CollisionMapInfo& info);
	void CheckCollisionLeft(CollisionMapInfo& info);

	Vector3 CornerPosition(const Vector3& center, Corner corner);
	Vector3 UDLRPositon(const Vector3& center, UDLR udlr);

	// 判定結果を反映して移動させる
	void ApplyResultAndMove(const CollisionMapInfo& info);
	/// @brief 天井に接触している場合の処理
	/// @param info
	void CeilingHit(const CollisionMapInfo& info);

	// 設置状態切り替え処理
	void SwitchOnGround(const CollisionMapInfo& info);

	/// @brief 壁に接触している場合の処理
	void WallHit(const CollisionMapInfo& info);

	/// @brief 描画
	void Draw();

	const WorldTransform& GetWorldTransform() const { return worldTransform_; };
	const Vector3& GetVelocity() const { return velocity_; };
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; };
	// ワールド座標を取得
	Vector3 GetWorldPosition();

	// AABBを取得する関数
	AABB GetAABB();
	// 衝突応答
	void OnCollision(const Enemy* enemy);
	// deathフラグのgetter
	bool IsDead() const { return isDead_; };

	// 通常行動更新
	void BehaviorRootUpdate();
	// 攻撃行動更新
	void BehaviorAttackUpdate(CollisionMapInfo collitionMapInfo);

	// 通常行動初期化
	void BehaviorRootInitialize();
	// 攻撃行動初期化
	void BehaviorAttackInitialize();

private:
	// ワールド変換データ
	WorldTransform worldTransform_;
	// モデル
	Model* model_ = nullptr;
	// カメラ
	Camera* camera_ = nullptr;
	// 速度
	Vector3 velocity_ = {};
	// 加速度
	static inline const float kAcceleration = 0.0625f;
	// 速度減衰率
	static inline const float kAttenuation = 0.25f;
	// 着地時の速度減衰率
	static inline const float kAttenuationLanding = 0.25f;
	// 壁接触時の速度減衰率
	static inline const float kAttenuationWall = 0.5f;
	// 最大速度制限
	static inline const float kLimitRunSpeed = 1.0f;
	// 方向
	LRDirection lrDirection_ = LRDirection::kRight;
	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	// 旋回タイマー 角度の補間するため
	float turnTimer_ = 0.0f;
	// 旋回時間<秒>
	static inline const float kTimeTurn = 0.3f;
	// 接地状態フラグ
	bool onGround_ = true;
	// 重力加速度（下方向）
	static inline const float kGravityAcceleration = 9.8f / 120.0f;
	// 最大落下速度
	static inline const float kLimitFallSpeed = 1.0f;
	// ジャンプ初速（上方向）
	static inline const float kJumpAcceleration = 1.0f;
	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;
	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
	// 当たり判定の空白部分
	static inline const float kBlank = 0.2f;
	// deathフラグ
	bool isDead_ = false;
	// 振る舞い
	Behavior behavior_ = Behavior::kRoot;
	// 次の振る舞いリクエスト
	Behavior behaviorRequest_ = Behavior::kUnKnown;
	// 攻撃ギミックの経過時間カウンター
	uint32_t attackParameter_ = 0;
	uint32_t attackTime_ = 10;
	AttackPhase attackPhase_;
	float attackSpeedX_ = 4.0f;
};