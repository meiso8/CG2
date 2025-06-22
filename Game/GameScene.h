#pragma once
#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "Fade.h"
#include "MapChipField.h"
#include "Player.h"
#include "Skydome.h"
#include <vector>
#include"../Header/DebugCamera.h"
#include"../Header/Input.h"

// ゲームシーン
class GameScene {

public:
    // 初期化
    void Initialize();

    // 更新
    void Update();
    void EnemyUpdate();
    void BlocksUpdate();
    void DeathParticlesUpdate();

    // 表示ブロックの生成
    void GenerateBlocks();
    // 全ての当たり判定を行う
    void CheckAllCollisions();
    /// @brief フェーズ変更
    void ChangePhase();

#ifdef _DEBUG
    void DebugCameraUpdate();
#endif

    // 描画
    void Draw();

    // デストラクタのゲッター
    bool IsFinished() const { return finished_; }

    // デストラクタ
    ~GameScene();

private:
    // ゲームのフェーズ（型）
    enum class Phase {
        kFadeIn,  // フェードイン
        kPlay,    // ゲームプレイ
        kDeath,   // デス演出
        kFadeOut, // フェードアウト
    };

    // ゲームの現在フェーズ（変数）
    Phase phase_;
    // フェード
    Fade* fade_ = nullptr;

    // テクスチャハンドル
    uint32_t modelTextureHandle_ = 0;
    // 3Dモデルデータ
    Model* model_ = nullptr;
    // カメラ
    Camera camera_;
    // カメラ操作
    CameraController* cameraController_ = nullptr;

    Input input_;

    // 自キャラ
    Player* player_ = nullptr;
    // 自キャラモデル
    Model* playerModel_ = nullptr;
    // 敵キャラを複数用意
    std::list<Enemy*> enemies_;
    //  敵キャラモデル
    Model* enemyModel_ = nullptr;
    // 敵の発生させる数
    static inline const int kEnemyMax = 3;

    // ブロックの3Dモデルデータ
    Model* blockModel_ = nullptr;
    // 二次元配列　ブロック用のワールドトランスフォーム
    std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;
    // マップチップフィールド
    MapChipField* mapChipField_;

    // 天球
    Skydome* skyDome_ = nullptr;
    // 天球モデル
    Model* skyDomeModel_ = nullptr;
    // 死亡時パーティクル
    Model* deathParticleModel_ = nullptr;
    DeathParticles* deathParticles_ = nullptr;

    // 終了フラグ
    bool finished_ = false;


#ifdef _DEBUG
    // デバッグカメラ有効
    bool isDebugCameraActive_ = false;
    // デバッグカメラの生成
    DebugCamera* debugCamera_ = nullptr;

#endif
};