#include<numbers>
#include"MyEngine.h"
#include"Game/GameScene.h"
#include"Game/TitleScene.h"
#include"Game/Merigora.h"
#include"Game/TextureIndex.h"
#include"Game/EndScene.h"
#include"Game/Building.h"

#define WIN_WIDTH 1280
#define WIN_HEIGHT 720

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

    MyEngine myEngine;
    myEngine.Create(L"クソゲーof the year 2025　ミラーキラー", WIN_WIDTH, WIN_HEIGHT);

    FPSCounter fpsCounter;

    Input* input = Input::GetInstance();
    //入力
    input->Initialize(myEngine.GetWC(), fpsCounter.GetFPS());

    srand(static_cast<unsigned int>(time(nullptr)));

#pragma region//XAudio全体の初期化と音声の読み込み
    //DirectX初期化処理の末尾に追加する
    //音声クラスの作成
    Sound sound;

    //音声読み込み SoundDataの変数を増やせばメモリが許す限りいくつでも読み込める。
    SoundData bgmData[2] = {
        sound.SoundLoad(L"resources/Sounds/dreamcore.mp3"),
        sound.SoundLoad(L"resources/Sounds/kiritan.mp3") };
    SoundData seData[4] = {
        sound.SoundLoad(L"resources/Sounds/broken.mp3"),
        sound.SoundLoad(L"resources/Sounds/pico.mp3") ,
        sound.SoundLoad(L"resources/Sounds/cracker.mp3"),
        sound.SoundLoad(L"resources/Sounds/poppo.mp3") };
    SoundData voiceData[3] =
    { sound.SoundLoad(L"resources/Sounds/voice1.wav"),
      sound.SoundLoad(L"resources/Sounds/voice2.wav"),
      sound.SoundLoad(L"resources/Sounds/voice3.wav") };


#pragma endregion

    DebugUI debugUI;


#pragma region//Camera

    bool isDebug = false;
    Camera camera;
    Transform cameraTransform{ { 1.0f, 1.0f, 1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,-10.0f } };
    camera.SetTransform(cameraTransform);
    camera.Initialize(static_cast<float>(WIN_WIDTH), static_cast<float>(WIN_HEIGHT), false);

    Camera cameraSprite;
    cameraSprite.Initialize(static_cast<float>(WIN_WIDTH), static_cast<float>(WIN_HEIGHT), true);

#pragma endregion

    Texture textures[TEXTURES] = {
     Texture(myEngine.GetDevice(), myEngine.GetCommandList()),
     Texture(myEngine.GetDevice(), myEngine.GetCommandList()),
     Texture(myEngine.GetDevice(), myEngine.GetCommandList()),
     Texture(myEngine.GetDevice(), myEngine.GetCommandList()),
     Texture(myEngine.GetDevice(), myEngine.GetCommandList()),
     Texture(myEngine.GetDevice(), myEngine.GetCommandList()),
      Texture(myEngine.GetDevice(), myEngine.GetCommandList()),
      Texture(myEngine.GetDevice(), myEngine.GetCommandList())
    };

    textures[WHITE].Load("resources/white1x1.png");
    textures[NUMBERS].Load("resources/numbers.png");
    textures[PRESS_SPACE].Load("resources/pressSpace.png");
    textures[SKY].Load("resources/sky.png");
    textures[CREDIT].Load("resources/credit.png");
    textures[PLAYER].Load("resources/player/player.png");
    textures[NICE].Load("resources/nice.png");
    textures[BRICK].Load("resources/brick.png");
    //ShaderResourceViewを作る
    ShaderResourceView srv[TEXTURES] = {};
    for (int i = 0; i < TEXTURES; ++i) {
        srv[i].Create(textures[i], i, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());
    }

    DrawGrid grid = DrawGrid(myEngine.GetDevice(), myEngine.GetModelConfig(0));

    Sprite numSprite[3];
    for (int i = 0; i < 3; ++i) {
        numSprite[i].Create(myEngine.GetDevice(), cameraSprite, myEngine.GetModelConfig(1));
        numSprite[i].SetSize(Vector2(80.0f, 80.0f));
        numSprite[i].SetTranslate({ 64.0f + i * numSprite[i].GetSize().x  ,64.0f,0.0f });
        numSprite[i].GetUVScale().x = 0.1f;
        numSprite[i].GetMaterial()->color = { 1.0f,0.0f,0.0f,1.0f };
    }

    const int spriteNum = 2;
    Sprite sprite[spriteNum];
    for (int i = 0; i < spriteNum; ++i) {
        sprite[i].Create(myEngine.GetDevice(), cameraSprite, myEngine.GetModelConfig(1));
    }

    sprite[0].SetSize(Vector2(256.0f, 256.0f));
    sprite[0].SetTranslate({ WIN_WIDTH - sprite[0].GetSize().x,WIN_HEIGHT - sprite[0].GetSize().y,0.0f });

    ModelData playerModelData[5] = {
        LoadObjeFile("resources/player", "body.obj"),
         LoadObjeFile("resources/player", "arm_L.obj"),
         LoadObjeFile("resources/player", "arm_R.obj"),
         LoadObjeFile("resources/player", "leg_L.obj"),
         LoadObjeFile("resources/player", "leg_R.obj")
    };

    ModelData hammerModelData = LoadObjeFile("resources/hammer", "hammer.obj");
    ModelData doveModelData = LoadObjeFile("resources/dove", "dove.obj");
    ModelData merigoraModelData = LoadObjeFile("resources/merigora", "merigora.obj");
    ModelData mirrorModelData = LoadObjeFile("resources/mirror", "mirror.obj");
    ModelData mirrorBallModelData = LoadObjeFile("resources/mirrorBall", "mirrorBall.obj");
    ModelData titleModelData = LoadObjeFile("resources/title", "title.obj");
    ModelData buildingModelData = LoadObjeFile("resources/building", "building.obj");
    ModelData benchModelData = LoadObjeFile("resources/building", "bench.obj");
    ModelData lightModelData = LoadObjeFile("resources/building", "light.obj");

    Model hammerModel(myEngine.GetModelConfig(0));
    hammerModel.Create(hammerModelData, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), HAMMER_MODEL);
    Model doveModel(myEngine.GetModelConfig(0));
    doveModel.Create(doveModelData, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), DOVE_MODEL);
    Model merigoraModel(myEngine.GetModelConfig(0));
    merigoraModel.Create(merigoraModelData, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), MERIGORA_MODEL);

    Vector4 worldColor = { 0.866f,0.627f,0.866f,1.0f };

    Building building[2];

    for (int i = 0; i < 2; ++i) {
        building[i].Init(myEngine, buildingModelData, BUILDING_MODEL);
        building[i].GetWorldTransform().translate_ = { 16.0f * (2 * i - 1),0.0f,16.0f };
        building[i].Update();
    }

    Building bench[3];
    for (int i = 0; i < 3; ++i) {
        bench[i].Init(myEngine, benchModelData, BUILDING_MODEL);
        bench[i].GetWorldTransform().translate_ = { -22.0f + (i * 6),0.0f,6.0f };
        bench[i].Update();
    }

    const int maxLight = 4;
    Building light[maxLight];

    for (int i = 0; i < maxLight; ++i) {
        light[i].Init(myEngine, lightModelData, BUILDING_MODEL);
        light[i].GetWorldTransform().translate_ = { 8.0f * (2 * i - 3),0.0f,10.0f };
        light[i].Update();
    }

    Merigora merigora;
    merigora.Init(merigoraModel, myEngine, doveModelData);

    std::unique_ptr<TitleScene> titleScene = std::make_unique<TitleScene>();
    titleScene->Init(myEngine, &hammerModel, sprite[1], titleModelData, camera, cameraSprite, worldColor);

    std::unique_ptr<GameScene> gameScene;

    std::unique_ptr<EndScene> endScene;
    MSG msg{};

    enum SCENE {
        TITLE,
        GAME,
        END
    };

    unsigned int scene = TITLE;

    // =============================================
    //ウィンドウのxボタンが押されるまでループ メインループ
    // =============================================
    while (msg.message != WM_QUIT) {

        //Windowにメッセージが来ていたら最優先で処理させる
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {

            //キーボード情報の取得開始
            input->InputInfoGet();

            //エンジンのアップデート
            myEngine.Update();

            fpsCounter.Update();

#pragma region //ゲームの処理

            merigora.Update();

            switch (scene) {
            case TITLE:
                titleScene->Update(sound, seData, bgmData);
                if (titleScene->IsTransition()) {
                    titleScene = nullptr;

                    gameScene = std::make_unique<GameScene>();
                    gameScene->Init(
                        myEngine,
                        &hammerModel,
                        &doveModel,
                        playerModelData,
                        mirrorModelData,
                        mirrorBallModelData,
                        numSprite, cameraSprite, camera, merigora);

                    scene = GAME;
                }
                break;
            case GAME:

                gameScene->Update(sound, bgmData, seData, voiceData);

                if (gameScene->IsTransition()) {
                    sound.SoundStop();
                    gameScene = nullptr;
                    endScene = std::make_unique<EndScene>();
                    endScene->Init(camera, cameraSprite, worldColor, myEngine);
                    scene = END;
                }

                if (isDebug) {

                    if (gameScene) {
                        debugUI.WorldTransformUpdate(building[1].GetWorldTransform());
                        debugUI.DebugMirror(gameScene->GetMirrors());
                        debugUI.HammerUpdate(gameScene->GetHammer());
                        debugUI.UpdatePlayer(gameScene->GetPlayer());
                        debugUI.SphereUpdate(gameScene->GetSphereMesh());
                        debugUI.DoveUpdate(gameScene->GetDove());
                        debugUI.CheckInt(gameScene->GetMirrorBreakCount());
                    }
                }
                break;
            case END:

                endScene->Update();


                if (!sound.IsPlaying() && !sound.IsActuallyPlaying()) {
                    sound.SoundPlay(bgmData[1], 0.5f, false);
                }

                if (endScene->IsTransition()) {

                    endScene = nullptr;
                    scene = TITLE;
                    sound.SoundStop();

                    titleScene = std::make_unique<TitleScene>();
                    titleScene->Init(myEngine, &hammerModel, sprite[1], titleModelData, camera, cameraSprite, worldColor);

                }
                break;
            }

#ifdef _DEBUG

            if (input->IsTriggerKey(DIK_P)) {
                //デバッグの切り替え
                isDebug = (isDebug) ? false : true;
            }

            if (isDebug) {
                debugUI.CheckDirectionalLight(myEngine.GetDirectionalLightData());
                debugUI.CheckFPS(fpsCounter);
                debugUI.SpriteUpdate(sprite[1]);
                debugUI.InputUpdate(*input);
                debugUI.Color(worldColor);
                debugUI.CameraUpdate(camera);
            }
#endif

            //カメラの切り替え処理
            if (isDebug) {
                //デバッグカメラに切り替え
                          //視点操作
                input->EyeOperation(camera);

                camera.Update();

            } else {

                switch (scene) {
                case TITLE:
                    if (titleScene != nullptr) {
                        titleScene->CameraUpdate();
                    }
                    break;
                case GAME:
                    if (gameScene != nullptr) {
                        gameScene->CameraUpdate();
                    }
                    break;
                case END:
                    endScene->CameraUpdate();
                    break;
                }

            }

#pragma region //描画


            myEngine.PreCommandSet(worldColor);

            //グリッドの描画
            if (isDebug) {
                grid.Draw(srv[WHITE], camera);
            }
            merigora.Draw(camera, srv);

            for (int i = 0; i < 2; ++i) {
                building[i].Draw(camera);
            }

            for (int i = 0; i < 3; ++i) {
                bench[i].Draw(camera);
            }

            for (int i = 0; i < maxLight; ++i) {
                light[i].Draw(camera);
            }

            switch (scene) {
            case TITLE:
                if (titleScene != nullptr) {
                    titleScene->Draw(srv);
                }
                break;
            case GAME:
                if (gameScene != nullptr) {
                    gameScene->Draw(srv, sprite);
                }
                break;
            case END:
                endScene->Draw(srv, sprite);
                break;
            }

            myEngine.PostCommandSet();

#pragma endregion

        }
    }

    myEngine.End();

    return 0;
}
