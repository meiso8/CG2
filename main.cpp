#include<numbers>
#include"MyEngine.h"

#define WIN_WIDTH 1280
#define WIN_HEIGHT 720

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

    MyEngine myEngine;
    myEngine.Create(WIN_WIDTH, WIN_HEIGHT);

    Input input;
    //入力
    input.Initialize(myEngine.GetWC());

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
    texture2.Load("resources/white2x2.png");

    //ShaderResourceViewを作る
    ShaderResourceView srv2 = {};
    srv2.Create(texture2, 2, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());

    DrawGrid grid = DrawGrid(myEngine.GetDevice(), camera, myEngine.GetModelConfig());

    Sprite sprite;
    sprite.Create(myEngine.GetDevice(), cameraSprite, myEngine.GetModelConfig());
    sprite.SetSize(Vector2(256.0f, 128.0f));

    Model model(myEngine.GetModelConfig());
    model.Create("resources/teapot", "teapot.obj", myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());

    Model model2(myEngine.GetModelConfig());
    model2.Create("resources/bunny", "bunny.obj", myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());

    Sphere sphere(myEngine.GetModelConfig());
    sphere.Create(myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());

    Vector3 scale = { 1.0f,1.0f,1.0f };
    Vector3 rotation = { 0.0f,0.0f,0.0f };
    Vector3 translation = { 0.0f,0.0f,0.0f };
    Matrix4x4 modelWorldMat = MakeAffineMatrix(scale, rotation, translation);

    MSG msg{};

    Vector2 speed = { 2.0f,2.0f };

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

#pragma region //ゲームの処理

#ifdef _DEBUG

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

            ImGui::Begin("Model");
            ImGui::SliderFloat3("translation", &translation.x, -10.0f, 10.0f);
            ImGui::SliderFloat3("rotation", &rotation.x, 0.0f, std::numbers::pi_v<float>*2.0f);
            ImGui::SliderFloat3("scale", &scale.x, 0.0f, 10.0f);
            ImGui::End();
            modelWorldMat = MakeAffineMatrix(scale, rotation, translation);

            ImGui::Begin("Sphere");
            ImGui::SliderFloat3("uvTranslate", &sphere.GetUVTransform().translate.x, -100.0f, 100.0f);
            ImGui::SliderFloat3("uvRotation", &sphere.GetUVTransform().rotate.x, 0.0f, std::numbers::pi_v<float>*2.0f);
            ImGui::SliderFloat3("uvScale", &sphere.GetUVTransform().scale.x, 0.0f, 10.0f);
            ImGui::End();

            sphere.UpdateUV();

            debugUI.SpriteUpdate(sprite);
            debugUI.ModelUpdate(model2);
            debugUI.InputUpdate(input);

#endif

            sprite.GetTranslateRef() += {speed.x, speed.y, 0.0f};

            if (sprite.GetTranslateRef().x > myEngine.GetWC().GetClientWidth() - sprite.GetSize().x || sprite.GetTranslateRef().x < 0.0f) {
                speed.x *= -1.0f;
            }

            if (sprite.GetTranslateRef().y > myEngine.GetWC().GetClientHeight() - sprite.GetSize().y || sprite.GetTranslateRef().y < 0.0f) {
                speed.y *= -1.0f;
            }

            sprite.Update();

#pragma region//視点操作

            input.EyeOperation(camera);

#pragma endregion

            if (input.IsTriggerKey(DIK_1)) {
                //音声再生
                sound.SoundPlay(soundData1);
            }

            if (input.IsTriggerKey(DIK_2)) {
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
            myEngine.PreCommandSet();

            grid.Draw(srv2);

            model.PreDraw();

            //model.Draw(MakeIdentity4x4(), camera);

            model2.Draw(MakeIdentity4x4(), camera);

            sphere.Draw(modelWorldMat, camera, srv);

            sprite.Draw(srv);

            myEngine.PostCommandSet();
#pragma endregion

        }
    }

    myEngine.End();

    return 0;
}
