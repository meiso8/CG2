#include<numbers>
#include"MyEngine.h"
#include"Game/GameScene.h"
#include"Game/TitleScene.h"
#include"Game/Merigora.h"

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
    SoundData bgmData[1] = { sound.SoundLoad(L"resources/Sounds/dreamcore.mp3") };
    SoundData seData[3] = {
        sound.SoundLoad(L"resources/Sounds/broken.mp3"),
        sound.SoundLoad(L"resources/Sounds/pico.mp3") ,
        sound.SoundLoad(L"resources/Sounds/cracker.mp3") };

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
                Texture(myEngine.GetDevice(), myEngine.GetCommandList()) ,
     Texture(myEngine.GetDevice(), myEngine.GetCommandList()) };

    textures[0].Load("resources/white1x1.png");
    textures[1].Load("resources/effect.png");
    textures[2].Load("resources/player/player.png");
    textures[3].Load("resources/sky.png");

    //ShaderResourceViewを作る
    ShaderResourceView srv[TEXTURES] = {};
    for (int i = 0; i < TEXTURES; ++i) {
        srv[i].Create(textures[i], i + 1, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());
    }
    DrawGrid grid = DrawGrid(myEngine.GetDevice(), myEngine.GetModelConfig(0));

    Sprite playerSprite;
    playerSprite.Create(myEngine.GetDevice(), cameraSprite, myEngine.GetModelConfig(1));
    playerSprite.SetSize(Vector2(256.0f, 256.0f));
    playerSprite.SetTranslate({ WIN_WIDTH - playerSprite.GetSize().x,WIN_HEIGHT - playerSprite.GetSize().y,0.0f });
    playerSprite.Update();

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
    playerModel.Create(playerModelData[0], myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), 5);
    assert(&playerModel);
    Model armLModel(myEngine.GetModelConfig(0));
    armLModel.Create(playerModelData[1], myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), 5);
    assert(&armLModel);
    Model armRModel(myEngine.GetModelConfig(0));
    armRModel.Create(playerModelData[2], myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), 5);
    assert(&armRModel);
    Model legLModel(myEngine.GetModelConfig(0));
    legLModel.Create(playerModelData[3], myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), 5);
    assert(&legLModel);
    Model legRModel(myEngine.GetModelConfig(0));
    legRModel.Create(playerModelData[4], myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), 5);
    assert(&legRModel);

    Model hammerModel(myEngine.GetModelConfig(0));
    hammerModel.Create(hammerModelData, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), 6);
    assert(&hammerModel);

    Model doveModel(myEngine.GetModelConfig(0));
    doveModel.Create(doveModelData, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), 8);
    assert(&doveModel);

    Model merigoraModel(myEngine.GetModelConfig(0));
    merigoraModel.Create(merigoraModelData, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), 9);
    assert(&merigoraModel);

    Vector4 worldColor = { 0.0f,0.0f,0.0f,1.0f };

    Vector2 speed = { 2.0f,2.0f };

    size_t currentIndex = 0;
    Vector4 colors[4] = {
        {1.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 1.0f, 1.0f}
    };

    float timer = 0.0f;

    TitleScene titleScene;
    titleScene.Init(myEngine, &hammerModel,mirrorModelData);

    GameScene gameScene;
    gameScene.Init(
        myEngine,
        &playerModel,
        &armLModel,
        &armRModel,
        &legLModel,
        &legRModel,
        &hammerModel,
        &doveModel,
        mirrorModelData);

    Merigora merigora;

    merigora.Init(merigoraModel, doveModel);

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

            timer++;
            float t = timer / 240.0f;

            if (t >= 1.0f) {
                currentIndex = (currentIndex + 1) % 4;
                timer = 0.0f;
                t = 0.0f;
            }

            worldColor = Lerp(colors[currentIndex], colors[(currentIndex + 1) % 4], t);

            merigora.Update();

            switch (scene) {
            case TITLE:
                titleScene.Update(sound, seData, bgmData);
                if (titleScene.IsTransition()) {
                    scene = GAME;
                }
                break;
            case GAME:
                gameScene.Update(sound, bgmData, seData, voiceData);
                break;
            case END:

                break;
            }

            myEngine.GetDirectionalLightData().direction = { 1.0f,0.0f,0.0f };

#ifdef _DEBUG

            {
                ImGui::Text("FPS : %d", fpsCounter.GetFPS());
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Pink");
                static char str0[128] = "Hello, world!";
                ImGui::InputText("input text", str0, IM_ARRAYSIZE(str0));

                if (ImGui::BeginMenu("file")) {
                    if (ImGui::MenuItem("newCreate")) { /* 処理 */ }
                    if (ImGui::BeginMenu("open")) {
                        if (ImGui::MenuItem("recentFile")) { /* 処理 */ }
                        if (ImGui::MenuItem("otherFile")) { /* 処理 */ }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }

            }

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

            debugUI.SpriteUpdate(playerSprite);
            debugUI.InputUpdate(*input);
            debugUI.Color(worldColor);
            debugUI.DebugMirror(gameScene.GetMirrors());
            debugUI.CameraUpdate(camera);
            debugUI.HammerUpdate(gameScene.GetHammer());
            debugUI.UpdatePlayer(gameScene.GetPlayer());
            debugUI.SphereUpdate(gameScene.GetSphereMesh());
            debugUI.DoveUpdate(gameScene.GetDove());
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
                    titleScene.CameraUpdate(camera);
                    break;
                case GAME:
                    gameScene.CameraUpdate(camera);
                    break;
                case END:

                    break;
                }

            }

#pragma region //描画


            myEngine.PreCommandSet(worldColor);

            //グリッドの描画
            grid.Draw(srv[WHITE], camera);

            merigora.Draw(camera);

            switch (scene) {
            case TITLE:

                titleScene.Draw(camera, srv, playerSprite);
                break;
            case GAME:
                gameScene.Draw(camera, srv, playerSprite);
                break;
            case END:

                break;
            }

            myEngine.PostCommandSet();

#pragma endregion

        }
    }

    myEngine.End();

    return 0;
}
