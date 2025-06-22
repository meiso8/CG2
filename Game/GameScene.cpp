#include "GameScene.h"
#include "../Header/math/AABB.h"
#include "WorldTransformUpdate.h"
#include"../Header/ImGuiClass.h"

const int winWidth = 1280;
const int winHeight = 720;

void GameScene::Initialize() {
    // メンバ変数への代入処理

    // ゲームプレイフェーズから開始
    phase_ = Phase::kFadeIn;

    fade_ = new Fade();
    fade_->Initialize();
    fade_->Start(Fade::Status::FadeIn, 1.0f);

    // カメラの初期化
    camera_.Initialize(winWidth, winHeight, false);
    camera_.SetFarZ(1000.0f);
    camera_.SetTarnslate({ 12.0f, 7.0f, -20.0f });

#ifdef _DEBUG
    // デバッグカメラの生成
    debugCamera_ = new DebugCamera();
    debugCamera_->Initialize(&input_, winWidth, winHeight);
#endif

    // マップチップフィールド
    mapChipField_ = new MapChipField;
    mapChipField_->LoadMapChipCsv("Resources/map3D.csv");

    // ブロックの生成関数
    GenerateBlocks();

    // ここにインゲームの初期化処理を書く
    modelTextureHandle_ = TextureManager::Load("texture0.jpg");

    // 天球モデルの生成 OBJからの生成
    playerModel_ = Model::CreateFromOBJ("player", true);
    // 自キャラの生成
    player_ = new Player();
    Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(4, 17);
    // 自キャラの初期化
    player_->Initialize(playerModel_, &camera_, &input_,playerPosition);
    player_->SetMapChipField(mapChipField_);

    // 敵のモデル生成
    enemyModel_ = Model::CreateFromOBJ("enemy", true);
    // 敵キャラ生成
    for (int32_t i = 0; i < kEnemyMax; ++i) {
        Enemy* newEnemy = new Enemy();
        Vector3 enemyPosition = { i * 5.0f + 10.0f, 1.0f, 0.0f };
        newEnemy->Initialize(enemyModel_, &camera_, enemyPosition);
        enemies_.push_back(newEnemy);
    }

    // 天球モデルの生成 OBJからの生成
    skyDomeModel_ = Model::CreateFromOBJ("world", true);
    skyDome_ = new Skydome();
    // 天球の生成
    skyDome_->Initialize(skyDomeModel_, &camera_);

    // パーティクルモデルの作成　
    deathParticleModel_ = Model::CreateFromOBJ("particle", false);

    // カメラ操作の初期化
    cameraController_ = new CameraController();
    cameraController_->Initialize(&camera_);
    cameraController_->SetTarget(player_);
    cameraController_->Reset();
    cameraController_->SetMovableArea({ 0.0f, 100.0f, 0.0f, 100.0f });
};

void GameScene::GenerateBlocks() {

    // ブロックの3Dモデルデータの生成
    blockModel_ = Model::CreateFromOBJ("cube", true);
    // 要素数
    const uint32_t kNumBlockVertical = 20;
    const uint32_t kNumBlockHorizontal = 100;

    // 要素数を変更する
    // 列数を設定（縦方向のブロック数）
    worldTransformBlocks_.resize(kNumBlockVertical);
    for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
        // 1列の要素数を設定（横方向のブロック数）
        worldTransformBlocks_[i].resize(kNumBlockHorizontal);
    }

    // ブロックの生成
    for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
        // キューブの生成
        for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {

            // 空きはnullptrが入る
            if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
                WorldTransform* worldTransform = new WorldTransform();
                worldTransform->Initialize();
                worldTransformBlocks_[i][j] = worldTransform;
                worldTransformBlocks_[i][j]->translate_ = mapChipField_->GetMapChipPositionByIndex(j, i);
            }
        }
    }
}

void GameScene::EnemyUpdate() {

    // 敵キャラの更新処理
    for (Enemy* newEnemy : enemies_) {
        if (!newEnemy)
            // ガード節と呼ぶ。
            continue;
        newEnemy->Update();
    }
}

void GameScene::BlocksUpdate() {

    // ブロックの更新処理
    for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
        for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {

            if (!worldTransformBlock)
                // ガード節と呼ぶ。
                continue;

            WorldTransformUpdate(*worldTransformBlock);
        }
    }
}

void GameScene::DeathParticlesUpdate() {

    // デスパーティクルの更新処理
    if (deathParticles_) {
        deathParticles_->Update();
    }
}
#ifdef _DEBUG

void GameScene::DebugCameraUpdate() {

    if (input_.IsTriggerKey(DIK_1)) {
        // スペースキーを押すとデバッグカメラに切り替える
        isDebugCameraActive_ = isDebugCameraActive_ ? false : true;
    }

    ImGui::SliderFloat3("camera translate", &camera_.translate_.x, -128.0f, 128.0f);

    // カメラの処理
    if (isDebugCameraActive_) {
        // デバッグカメラの更新
        debugCamera_->Update();
        camera_.SetViewMatrix( debugCamera_->GetViewMatrix());
        camera_.SetProjectionMatrix(debugCamera_->GetProjectionMatrix());
        // ビュープロジェクション行列の転送
        camera_.TransferMatrix();

    } else {
        // ビュープロジェクション行列の更新と転送
        camera_.Update();
    }
};

#endif

void GameScene::CheckAllCollisions() {

#pragma region // 自キャラと敵キャラの当たり判定
    // 自キャラと敵キャラの当たり判定

    AABB aabb1, aabb2;

    aabb1 = player_->GetAABB();

    for (Enemy* enemy : enemies_) {
        aabb2 = enemy->GetAABB();

        // AABB同士の交差判定
        if (IsCollision(aabb1, aabb2)) {
            // 自キャラ衝突時コールバックを呼び出す
            player_->OnCollision(enemy);
            // 敵弾の衝突時コールバックを呼び出す
            enemy->OnCollision(player_);
        }
    }

#pragma endregion

#pragma region // 自キャラとアイテムの当たり判定
    // 自キャラとアイテムの当たり判定
#pragma endregion

#pragma region // 自弾と敵キャラの当たり判定
// 自弾と敵キャラの当たり判定
#pragma endregion
}

void GameScene::ChangePhase() {

    switch (phase_) {
    case Phase::kPlay:
        // ゲームプレイフェーズの処理
        break;
    case Phase::kDeath:
        // デス演出フェーズの処理
        break;
    }
}

void GameScene::Update() {

    // ここにインゲームの更新処理を書く

    switch (phase_) {
    case Phase::kFadeIn:

        fade_->Update();

        if (fade_->IsFinished()) {
            phase_ = Phase::kPlay;
        }

        break;
    case Phase::kPlay:
        // ゲームプレイフェーズの処理

        // 天球の更新処理
        skyDome_->Update();

        // 自キャラの更新処理
        player_->Update();

        // 敵の更新
        EnemyUpdate();

        // カメラコントローラーの更新
        cameraController_->Update();

#ifdef _DEBUG
        DebugCameraUpdate();
#endif
        // ブロックの更新
        BlocksUpdate();

        // 全ての当たり判定を行う
        CheckAllCollisions();

        if (player_->IsDead()) {
            // 死亡演出フェーズに切り替え
            phase_ = Phase::kDeath;
            // 自キャラの座標を取得
            const Vector3& deathParticlePosition = player_->GetWorldPosition();
            // 仮の生成処理。後で消す
            deathParticles_ = new DeathParticles;
            deathParticles_->Initialize(deathParticleModel_, &camera_, deathParticlePosition);
        }

        break;
    case Phase::kDeath:

        // デス演出フェーズの処理
        // 天球の更新処理
        skyDome_->Update();

        // 敵の更新
        EnemyUpdate();

        // デスパーティクルの更新
        DeathParticlesUpdate();

#ifdef _DEBUG
        DebugCameraUpdate();
#endif
        // ブロックの更新
        BlocksUpdate();

        if (deathParticles_ && deathParticles_->IsFinished()) {
            fade_->Start(Fade::Status::FadeOut, 1.0f);
            phase_ = Phase::kFadeOut;
        }

        break;

    case Phase::kFadeOut:

        fade_->Update();

        if (fade_->IsFinished()) {
            finished_ = true;
        }

        break;
    }
};

void GameScene::Draw() {

    // 天球の描画
    skyDome_->Draw();

    if (!player_->IsDead()) {
        // 自キャラの描画
        player_->Draw();
    }

    // 敵キャラの描画
    for (Enemy* newEnemy : enemies_) {
        if (!newEnemy)
            // ガード節と呼ぶ。
            continue;
        newEnemy->Draw();
    }


    // 3Dモデルの描画前処理
    Model::PreDraw(dxCommon->GetCommandList());

    // ブロックの描画
    for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
        for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {

            if (!worldTransformBlock)
                // ガード節と呼ぶ。
                continue;

            blockModel_->Draw(*worldTransformBlock, camera_);
        }
    }


    Model::PostDraw();

    // デスパーティクルの描画処理
    if (deathParticles_) {
        deathParticles_->Draw();
    }

    switch (phase_) {
    case Phase::kFadeIn:
        fade_->Draw(*dxCommon->GetCommandList());
        break;
    case Phase::kPlay:
        break;
    case Phase::kDeath:
        break;
    case Phase::kFadeOut:

        fade_->Draw(*dxCommon->GetCommandList());
        break;
    }
}

GameScene::~GameScene() {
    delete player_;
    delete playerModel_;
    delete enemyModel_;
    delete blockModel_;
    delete skyDome_;
    delete skyDomeModel_;
    delete mapChipField_;
    delete cameraController_;
    delete deathParticleModel_;
    // パーティクルの解放
    if (deathParticles_) {
        delete deathParticles_;
    }

#ifdef _DEBUG
    delete debugCamera_;
#endif

    // 配列内の要素を1個ずつ解放　&をつけることで二次元vector内のデータ自体に対して処理を行う。
    for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
        for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
            delete worldTransformBlock;
        }
    }

    // 配列から要素(ポインタの残骸)を一掃している。
    worldTransformBlocks_.clear();

    // 敵delete
    for (Enemy* newEnemy : enemies_) {
        delete newEnemy;
    }

    // 敵のポインタの残骸を一掃
    enemies_.clear();
}