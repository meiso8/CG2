#include<numbers>
#include"MyEngine.h"

#include"FPSCounter.h"

#define WIN_WIDTH 1280
#define WIN_HEIGHT 720

#include<algorithm>

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

    MyEngine myEngine;
    myEngine.Create(L"EEZEngine", WIN_WIDTH, WIN_HEIGHT);

    FPSCounter fpsCounter;

    Input input;
    //入力
    input.Initialize(myEngine.GetWC(), fpsCounter.GetFPS());

#pragma region//XAudio全体の初期化と音声の読み込み
    //DirectX初期化処理の末尾に追加する
    //音声クラスの作成
    Sound sound;
    sound.Initialize();

    //ここはゲームによって異なる
     //音声読み込み SoundDataの変数を増やせばメモリが許す限りいくつでも読み込める。
    SoundData soundData1 = sound.SoundLoad(L"resources/Alarm01.wav");
    SoundData soundData2 = sound.SoundLoad(L"resources/dreamcore.mp3");

#pragma endregion

    DebugUI debugUI;


#pragma region//Camera

    bool isDebug = false;

    Camera camera;
    DebugCamera debugCamera;

    Camera cameraSprite;
    cameraSprite.Initialize(static_cast<float>(WIN_WIDTH), static_cast<float>(WIN_HEIGHT), true);

#pragma endregion

    Transform cameraTransform{ { 1.0f, 1.0f, 1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,-5.0f } };
    camera.SetTransform(cameraTransform);
    camera.Initialize(static_cast<float>(WIN_WIDTH), static_cast<float>(WIN_HEIGHT), false);

    debugCamera.Initialize(&input, static_cast<float>(WIN_WIDTH), static_cast<float>(WIN_HEIGHT));

    Texture texture = Texture(myEngine.GetDevice(), myEngine.GetCommandList());
    texture.Load("resources/dvd.png");

    //ShaderResourceViewを作る
    ShaderResourceView srv = {};
    srv.Create(texture, 1, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());

    Texture texture2 = Texture(myEngine.GetDevice(), myEngine.GetCommandList());
    texture2.Load("resources/white1x1.png");

    //ShaderResourceViewを作る
    ShaderResourceView srv2 = {};
    srv2.Create(texture2, 2, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());

    Texture texture3 = Texture(myEngine.GetDevice(), myEngine.GetCommandList());
    texture3.Load("resources/effect.png");

    //ShaderResourceViewを作る
    ShaderResourceView srv3 = {};
    srv3.Create(texture3, 4, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());


    DrawGrid grid = DrawGrid(myEngine.GetDevice(), camera, myEngine.GetModelConfig());

    Sprite sprite;
    sprite.Create(myEngine.GetDevice(), cameraSprite, myEngine.GetModelConfig());
    sprite.SetSize(Vector2(256.0f, 128.0f));

    Model model(myEngine.GetModelConfig());
    model.Create("resources/teapot", "teapot.obj", myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());

    Model bunnyModel(myEngine.GetModelConfig());
    bunnyModel.Create("resources/bunny", "bunny.obj", myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());

    Sphere sphere(myEngine.GetModelConfig());
    sphere.Create(myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());

    Vector3 scale[3] = {};
    Vector3 rotation[3] = {};
    Vector3 translation[3] = {};
    Matrix4x4 worldMat[3] = {};

    for (int i = 0; i < 3; ++i) {
        scale[i] = { 1.0f,1.0f,1.0f };
        rotation[i] = { 0.0f,0.0f,0.0f };
        translation[i] = { (i - 1) * 5.0f,0.0f,0.0f };
        worldMat[i] = MakeAffineMatrix(scale[i], rotation[i], translation[i]);
    }

    scale[1] = { 10.0f,10.0f,10.0f };

    MSG msg{};

    bool isExpansion = false;
    float expansionSpeed = 1.0f / 120.0f;

    Vector4 worldColor = { 0.0f,0.0f,0.0f,1.0f };

    Vector2 speed = { 2.0f,2.0f };
    size_t currentIndex = 0;
    Vector4 colors[4] = { {1.0f,0.0f,0.0f,1.0f}, {0.0f,1.0f,0.0f,1.0f}, {0.0f,0.0f,1.0f,1.0f}, {1.0f,0.0f,1.0f,1.0f} };
    float timer = 0.0f;

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
            input.InputInfoGet();

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

#ifdef _DEBUG

            {

                ImGui::Text("FPS : %d", fpsCounter.GetFPS());
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Pink");
                static char str0[128] = "Hello, world!";
                ImGui::InputText("input text", str0, IM_ARRAYSIZE(str0));

                const char* items[] = { "1", "2", "3" };
                static int item_current = 0;

                ImGui::Combo("Sprite", &item_current, items, IM_ARRAYSIZE(items));

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

            ImGui::Begin("WorldMatrix");
            ImGui::SliderFloat3("0:translation", &translation[0].x, -10.0f, 10.0f);
            ImGui::SliderFloat3("0:rotation", &rotation[0].x, 0.0f, std::numbers::pi_v<float>*2.0f);
            ImGui::SliderFloat3("0:scale", &scale[0].x, 0.0f, 10.0f);

            ImGui::SliderFloat3("1:translation", &translation[1].x, -10.0f, 10.0f);
            ImGui::SliderFloat3("1:rotation", &rotation[1].x, 0.0f, std::numbers::pi_v<float>*2.0f);
            ImGui::SliderFloat3("1:scale", &scale[1].x, 0.0f, 10.0f);

            ImGui::SliderFloat3("2:translation", &translation[2].x, -10.0f, 10.0f);
            ImGui::SliderFloat3("2:rotation", &rotation[2].x, 0.0f, std::numbers::pi_v<float>*2.0f);
            ImGui::SliderFloat3("2:scale", &scale[2].x, 0.0f, 10.0f);

            ImGui::End();

            rotation[0].y += 1.0f / 60.0f;

            worldMat[0] = MakeAffineMatrix(scale[0], rotation[0], translation[0]);
            worldMat[1] = MakeAffineMatrix(scale[1], rotation[1], translation[1]);
            worldMat[2] = MakeAffineMatrix(scale[2], rotation[2], translation[2]);

            debugUI.SphereUpdate(sphere);
            debugUI.SpriteUpdate(sprite);
            debugUI.ModelUpdate(bunnyModel);
            debugUI.InputUpdate(input);

            sphere.SetUVScale({ 1.0f,50.0f,1.0f });
            sphere.GetUVTransform().translate += { 0.0f, 1.0f / 60.0f, 0.0f };
            sphere.UpdateUV();
            sphere.SetColor(Lerp(colors[currentIndex], colors[(currentIndex + 1) % 4], t));

            bunnyModel.GetWaveData(0).amplitude = 1.0f / 64.0f;
            bunnyModel.GetWaveData(0).time += 1.0f / 60.0f;

            if (isExpansion) {
                bunnyModel.GetExpansionData().expansion += expansionSpeed;

                bool isSmall = bunnyModel.GetExpansionData().expansion <= 0.0f;

                if (isSmall) {
                    isExpansion = false;
                    bunnyModel.GetExpansionData().expansion = 0.0f;
                }

                if (bunnyModel.GetExpansionData().expansion >= 1.0f || isSmall) {
                    expansionSpeed *= -1.0f;

                }
            }

#endif


            sprite.SetColor(Lerp(colors[currentIndex], colors[(currentIndex + 1) % 4], t));
            sprite.GetTranslateRef() += {speed.x, speed.y, 0.0f};

            if (sprite.GetTranslateRef().x > myEngine.GetWC().GetClientWidth() - sprite.GetSize().x || sprite.GetTranslateRef().x < 0.0f) {
                speed.x *= -1.0f;
            }

            if (sprite.GetTranslateRef().y > myEngine.GetWC().GetClientHeight() - sprite.GetSize().y || sprite.GetTranslateRef().y < 0.0f) {
                speed.y *= -1.0f;
            }

            sprite.Update();

            //視点操作
            input.EyeOperation(camera);

            if (input.IsTriggerKey(DIK_1)) {
                //音声再生
                sound.SoundPlay(soundData1);
            }

            if (input.IsTriggerKey(DIK_2)) {
                //音声再生
                sound.SoundPlay(soundData2);
                isExpansion = true;
            }

            if (input.IsTriggerKey(DIK_3)) {
                //音声再生
                sound.SoundPlay(soundData2);
            }

            if (input.IsTriggerKey(DIK_SPACE)) {
                //デバッグの切り替え
                isDebug = (isDebug) ? false : true;
            }

            if (input.IsTriggerKey(DIK_RETURN)) {
                debugCamera.SetIsOrthographic(true);
            }

            //カメラの切り替え処理
            if (isDebug) {
                //デバッグカメラに切り替え
                camera.SetViewMatrix(debugCamera.GetViewMatrix());
                camera.SetProjectionMatrix(debugCamera.GetProjectionMatrix());
                debugCamera.Update();

            } else {
                //カメラの更新処理
                camera.Update();
            }

#pragma region //描画


            myEngine.PreCommandSet(worldColor);

            grid.Draw(srv2);

            model.PreDraw();

            model.Draw(worldMat[0], camera);
            bunnyModel.Draw(worldMat[1], camera);
            sphere.Draw(worldMat[2], camera, srv3);

            sprite.Draw(srv);


            myEngine.PostCommandSet();
#pragma endregion

        }
    }

    myEngine.End();

    return 0;
}
