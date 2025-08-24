#include<numbers>
#include"MyEngine.h"
#include"Game/GameScene.h"
#include"Game/TitleScene.h"
#include"Game/Merigora.h"
#include"Game/TextureIndex.h"
#include"Game/EndScene.h"

#define WIN_WIDTH 1280
#define WIN_HEIGHT 720



//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

    MyEngine myEngine;
    myEngine.Create(L"ミラーキラー", WIN_WIDTH, WIN_HEIGHT);

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
    SoundData bgmData[2] = { sound.SoundLoad(L"resources/Sounds/dreamcore.mp3"),
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

    const int maxTexture = 5;

    Texture textures[maxTexture] = {
        Texture(myEngine.GetDevice(), myEngine.GetCommandList()),
            Texture(myEngine.GetDevice(), myEngine.GetCommandList()),
                Texture(myEngine.GetDevice(), myEngine.GetCommandList()) ,
     Texture(myEngine.GetDevice(), myEngine.GetCommandList()),Texture(myEngine.GetDevice(), myEngine.GetCommandList()) };

    textures[WHITE].Load("resources/white1x1.png");
    textures[NUMBERS].Load("resources/numbers.png");
    textures[PRESS_SPACE].Load("resources/pressSpace.png");
    textures[SKY_MODEL].Load("resources/sky.png"),
        textures[PLAYER].Load("resources/player/player.png");

    //ShaderResourceViewを作る
    ShaderResourceView srv[maxTexture] = {};
    for (int i = 0; i < maxTexture; ++i) {
        srv[i].Create(textures[i], i, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());
    }
    DrawGrid grid = DrawGrid(myEngine.GetDevice(), myEngine.GetModelConfig(0));

    Sprite numSprite[3];
    for (int i = 0; i < 3; ++i) {
        numSprite[i].Create(myEngine.GetDevice(), cameraSprite, myEngine.GetModelConfig(1));
        numSprite[i].SetSize(Vector2(80.0f, 80.0f));
        numSprite[i].SetTranslate({ numSprite[i].GetSize().x + i * 80.0f ,numSprite[i].GetSize().y,0.0f });
        numSprite[i].GetUVScale().x = 0.1f;
        numSprite[i].Update();
    }

    Sprite sprite[2];
    for (int i = 0; i < 2; ++i) {
        sprite[i].Create(myEngine.GetDevice(), cameraSprite, myEngine.GetModelConfig(1));
    }

    sprite[0].SetSize(Vector2(256.0f, 256.0f));
    sprite[0].SetTranslate({ WIN_WIDTH - sprite[0].GetSize().x,WIN_HEIGHT - sprite[0].GetSize().y,0.0f });
    sprite[0].Update();

    sprite[1].SetSize(Vector2(901.0f, 127.0f));
    sprite[1].SetTranslate({ myEngine.GetWC().GetClientWidth() / 2.0f - sprite[1].GetSize().x / 2.0f, myEngine.GetWC().GetClientHeight() - sprite[1].GetSize().y,0.0f });
    sprite[1].Update();

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

    Model playerModel(myEngine.GetModelConfig(0));
    playerModel.Create(playerModelData[0], myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), PLAYER_MODEL);
    assert(&playerModel);
    Model armLModel(myEngine.GetModelConfig(0));
    armLModel.Create(playerModelData[1], myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), PLAYER_MODEL);
    assert(&armLModel);
    Model armRModel(myEngine.GetModelConfig(0));
    armRModel.Create(playerModelData[2], myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), PLAYER_MODEL);
    assert(&armRModel);
    Model legLModel(myEngine.GetModelConfig(0));
    legLModel.Create(playerModelData[3], myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), PLAYER_MODEL);
    assert(&legLModel);
    Model legRModel(myEngine.GetModelConfig(0));
    legRModel.Create(playerModelData[4], myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), PLAYER_MODEL);
    assert(&legRModel);

    Model hammerModel(myEngine.GetModelConfig(0));
    hammerModel.Create(hammerModelData, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), HAMMER_MODEL);
    assert(&hammerModel);

    Model doveModel(myEngine.GetModelConfig(0));
    doveModel.Create(doveModelData, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), DOVE_MODEL);
    assert(&doveModel);

    Model merigoraModel(myEngine.GetModelConfig(0));
    merigoraModel.Create(merigoraModelData, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), MERIGORA_MODEL);
    assert(&merigoraModel);

    Vector4 worldColor = { 0.866f,0.627f,0.866f,1.0f };

    Vector2 speed = { 2.0f,2.0f };

    float timer = 0.0f;

    std::unique_ptr<TitleScene> titleScene = std::make_unique<TitleScene>();
    titleScene->Init(myEngine, &hammerModel, mirrorModelData, sprite[1]);

    std::unique_ptr<GameScene> gameScene = std::make_unique<GameScene>();
    gameScene->Init(myEngine, &playerModel, &armLModel, &armRModel, &legLModel,
        &legRModel, &hammerModel, &doveModel, mirrorModelData, numSprite);

    std::unique_ptr<EndScene> endScene = std::make_unique<EndScene>();

    Merigora merigora;

    merigora.Init(merigoraModel, myEngine, doveModelData);

    Vector3 direction = { 0.0f,0.0f,1.0f };

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
                    titleScene.reset(); // 自動でdeleteが呼び出される
                    scene = GAME;
                    titleScene = nullptr;

                    gameScene = std::make_unique<GameScene>();

                    gameScene->Init(
                        myEngine,
                        &playerModel,
                        &armLModel,
                        &armRModel,
                        &legLModel,
                        &legRModel,
                        &hammerModel,
                        &doveModel,
                        mirrorModelData, numSprite);
                }
                break;
            case GAME:
                gameScene->Update(sound, bgmData, seData, voiceData);
                if (gameScene->IsTransition()) {
                    gameScene.reset();

                    gameScene = nullptr;
                    scene = END;
                    sound.SoundStop();
                    endScene = std::make_unique<EndScene>();
                }
                break;
            case END:

                endScene->Update(worldColor, myEngine.GetDirectionalLightData().color);

                if (!sound.IsPlaying() && !sound.IsActuallyPlaying()) {
                    sound.SoundPlay(bgmData[1], 1.0f, false);
                }

                if (input->IsTriggerKey(DIK_SPACE)) {
                    endScene->IsTransition() = true;

                    if (endScene->IsTransition()) {
                        endScene.reset();

                        endScene = nullptr;
                        scene = TITLE;
                        sound.SoundStop();

                        titleScene = std::make_unique<TitleScene>();
                        titleScene->Init(myEngine, &hammerModel, mirrorModelData, sprite[1]);

                    }
            
                }

                break;
            }

            myEngine.GetDirectionalLightData().direction = { 1.0f,0.0f,0.0f };

#ifdef _DEBUG

            ImGui::Begin("Debug");
            ImGui::Text("FPS : %d", fpsCounter.GetFPS());
            ImGui::End();

            {

                ImGui::Begin("DirectionalLight");
                ImGui::ColorEdit4("color", &myEngine.GetDirectionalLightData().color.x);
                ImGui::SliderFloat3("direction", &direction.x, -1.0f, 1.0f);//後で正規化する
                myEngine.GetDirectionalLightData().direction = Normalize(direction);
                ImGui::DragFloat("intensity", &myEngine.GetDirectionalLightData().intensity);

                const char* lights[] = { "NONE", "LambertianReflectance", "HalfLambert" };

                static int light_current = 2;

                ImGui::Combo("LightMode", &light_current, lights, IM_ARRAYSIZE(lights));
                playerModel.GetMaterial()->lightType = light_current % 3;

                ImGui::End();
            }

            debugUI.SpriteUpdate(numSprite[0]);
            debugUI.InputUpdate(*input);
            debugUI.Color(worldColor);
            debugUI.CameraUpdate(camera);

            if (gameScene) {
                debugUI.DebugMirror(gameScene->GetMirrors());
                debugUI.HammerUpdate(gameScene->GetHammer());
                debugUI.UpdatePlayer(gameScene->GetPlayer());
                debugUI.SphereUpdate(gameScene->GetSphereMesh());
                debugUI.DoveUpdate(gameScene->GetDove());
                debugUI.CheckInt(gameScene->GetMirrorBreakCount());
            }
#endif

            if (input->IsTriggerKey(DIK_P)) {
                //デバッグの切り替え
                isDebug = (isDebug) ? false : true;
            }

            //カメラの切り替え処理
            if (isDebug) {
                //デバッグカメラに切り替え
                          //視点操作
                input->EyeOperation(camera);

                camera.Update();

            } else {

                switch (scene) {
                case TITLE:
                    titleScene->CameraUpdate(camera);
                    break;
                case GAME:

                    gameScene->CameraUpdate(camera);
                    break;
                case END:
         endScene->CameraUpdate(camera);


                    break;
                }

            }

#pragma region //描画


            myEngine.PreCommandSet(worldColor);

            //グリッドの描画
            grid.Draw(srv[WHITE], camera);
            merigora.Draw(camera, srv);

            switch (scene) {
            case TITLE:
                titleScene->Draw(camera, srv);
                break;
            case GAME:
                gameScene->Draw(camera, srv, sprite);
                break;
            case END:
                endScene->Draw(camera, srv, sprite[1]);
     
                break;
            }

            myEngine.PostCommandSet();

#pragma endregion

        }
    }

    myEngine.End();

    return 0;
}
