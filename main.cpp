#include<numbers>
#include"MyEngine.h"
#include"Game/GameScene.h"

#define WIN_WIDTH 1280
#define WIN_HEIGHT 720

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

    MyEngine myEngine;
    myEngine.Create(L"EEZEngine", WIN_WIDTH, WIN_HEIGHT);

    FPSCounter fpsCounter;

    Input* input = Input::GetInstance();
    //入力
    input->Initialize(myEngine.GetWC(), fpsCounter.GetFPS());

    srand(static_cast<unsigned int>(time(nullptr)));


#pragma region//XAudio全体の初期化と音声の読み込み
    //DirectX初期化処理の末尾に追加する
    //音声クラスの作成
    Sound sound;
    sound.Initialize();

    //音声読み込み SoundDataの変数を増やせばメモリが許す限りいくつでも読み込める。
    SoundData soundData1 = sound.SoundLoad(L"resources/Sounds/broken.mp3");
    SoundData soundData2 = sound.SoundLoad(L"resources/Sounds/dreamcore.mp3");

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

    DebugCamera debugCamera;
    debugCamera.Initialize(input, static_cast<float>(WIN_WIDTH), static_cast<float>(WIN_HEIGHT));
#pragma endregion

    //Texture texture = Texture(myEngine.GetDevice(), myEngine.GetCommandList());
    //texture.Load("resources/dvd.png");

    ////ShaderResourceViewを作る
    //ShaderResourceView srv = {};
    //srv.Create(texture, 1, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());

    Texture texture2 = Texture(myEngine.GetDevice(), myEngine.GetCommandList());
    texture2.Load("resources/white1x1.png");

    //ShaderResourceViewを作る
    ShaderResourceView srv2 = {};
    srv2.Create(texture2, 1, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());

    Texture texture3 = Texture(myEngine.GetDevice(), myEngine.GetCommandList());
    texture3.Load("resources/effect.png");

    //ShaderResourceViewを作る
    ShaderResourceView srv3 = {};
    srv3.Create(texture3, 2, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());


    DrawGrid grid = DrawGrid(myEngine.GetDevice(), camera, myEngine.GetModelConfig(0));

    //Sprite sprite;
    //sprite.Create(myEngine.GetDevice(), cameraSprite, myEngine.GetModelConfig(1));
    //sprite.SetSize(Vector2(256.0f, 128.0f));


    Texture texture4 = Texture(myEngine.GetDevice(), myEngine.GetCommandList());
    texture4.Load("resources/player/player.png");

    //ShaderResourceViewを作る
    ShaderResourceView srv4 = {};
    srv4.Create(texture4, 3, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());

    Sprite playerSprite;
    playerSprite.Create(myEngine.GetDevice(), cameraSprite, myEngine.GetModelConfig(1));
    playerSprite.SetSize(Vector2(256.0f, 256.0f));
    playerSprite.SetTranslate({ WIN_WIDTH - playerSprite.GetSize().x,WIN_HEIGHT - playerSprite.GetSize().y,0.0f });
    playerSprite.Update();

    Model bunnyModel(myEngine.GetModelConfig(0));
    bunnyModel.Create("resources", "bunny.obj", myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), 4);
    assert(&bunnyModel);

    Vector4 worldColor = { 0.0f,0.0f,0.0f,1.0f };

    Vector2 speed = { 2.0f,2.0f };

    size_t currentIndex = 0;
    Vector4 colors[4] = {
        {1.0f, 0.8f, 0.6f, 1.0f}, // 朝方：暖かいオレンジ色
        {0.6f, 0.9f, 1.0f, 1.0f}, // 昼：爽やかな水色
        {1.0f, 0.5f, 0.3f, 1.0f}, // 夕方：夕焼けオレンジ
        {0.1f, 0.1f, 0.3f, 1.0f}  // 深夜：濃いブルー
    };

    float timer = 0.0f;

    GameScene gameScene;
    gameScene.Init(myEngine, &bunnyModel);


    MSG msg{};

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
            gameScene.Update(sound, soundData1, soundData2, voiceData);


#ifdef _DEBUG


            //視点操作
            input->EyeOperation(camera);

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

            //ImGui::Begin("WorldMatrix");
            //const char* items[] = { "SPHERE" };
            //debugUI.WorldMatrixUpdate(scale, rotation, translation, items[0]);
            //ImGui::End();

            {
                Vector3 direction = myEngine.GetDirectionalLightData().direction;
                ImGui::Begin("DirectionalLight");
                ImGui::ColorEdit4("color", &myEngine.GetDirectionalLightData().color.x);
                ImGui::SliderFloat3("direction", &direction.x, -1.0f, 1.0f);//後で正規化する
                myEngine.GetDirectionalLightData().direction = Normalize(direction);
                ImGui::DragFloat("intensity", &myEngine.GetDirectionalLightData().intensity);

                const char* lights[] = { "NONE", "LambertianReflectance", "HalfLambert" };

                static int light_current = 2;

                ImGui::Combo("LightMode", &light_current, lights, IM_ARRAYSIZE(lights));
                bunnyModel.GetMaterial()->lightType = light_current % 3;

                ImGui::End();
            }

            debugUI.SpriteUpdate(playerSprite);
            debugUI.InputUpdate(*input);
            debugUI.Color(worldColor);
            debugUI.DebugMirror(gameScene.GetMirrors());

#endif

            if (input->IsTriggerKey(DIK_RETURN)) {
                debugCamera.SetIsOrthographic(true);
            }

            if (input->IsTriggerKey(DIK_P)) {
                //デバッグの切り替え
                isDebug = (isDebug) ? false : true;
            }

            //カメラの切り替え処理
            if (isDebug) {
                //デバッグカメラに切り替え
                camera.SetViewMatrix(debugCamera.GetViewMatrix());
                camera.SetProjectionMatrix(debugCamera.GetProjectionMatrix());
                debugCamera.Update();

            } else {
                gameScene.CameraUpdate(camera);
            }

#pragma region //描画


            myEngine.PreCommandSet(worldColor);

            //グリッドの描画
            grid.Draw(srv2);

            gameScene.Draw(camera, srv3, srv4, playerSprite);

            myEngine.PostCommandSet();
#pragma endregion

        }
    }



    myEngine.End();

    return 0;
}
