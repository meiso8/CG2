#include<numbers>
#include"MyEngine.h"



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

    bool isSound = false;
    bool isVoiceSound = false;

#pragma endregion

    DebugUI debugUI;


#pragma region//Camera

    bool isDebug = false;

    Camera camera;
    DebugCamera debugCamera;

    Camera cameraSprite;
    cameraSprite.Initialize(static_cast<float>(WIN_WIDTH), static_cast<float>(WIN_HEIGHT), true);

#pragma endregion

    Transform cameraTransform{ { 1.0f, 1.0f, 1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,-10.0f } };
    camera.SetTransform(cameraTransform);
    camera.Initialize(static_cast<float>(WIN_WIDTH), static_cast<float>(WIN_HEIGHT), false);

    debugCamera.Initialize(input, static_cast<float>(WIN_WIDTH), static_cast<float>(WIN_HEIGHT));

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
    srv3.Create(texture3, 3, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());


    DrawGrid grid = DrawGrid(myEngine.GetDevice(), camera, myEngine.GetModelConfig(0));

    Sprite sprite;
    sprite.Create(myEngine.GetDevice(), cameraSprite, myEngine.GetModelConfig(1));
    sprite.SetSize(Vector2(256.0f, 128.0f));


    Texture texture4 = Texture(myEngine.GetDevice(), myEngine.GetCommandList());
    texture4.Load("resources/player/player.png");

    //ShaderResourceViewを作る
    ShaderResourceView srv4 = {};
    srv4.Create(texture4, 6, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());

    Sprite playerSprite;
    playerSprite.Create(myEngine.GetDevice(), cameraSprite, myEngine.GetModelConfig(1));
    playerSprite.SetSize(Vector2(256.0f, 256.0f));
    playerSprite.SetTranslate({ WIN_WIDTH - playerSprite.GetSize().x,WIN_HEIGHT - playerSprite.GetSize().y,0.0f });
    playerSprite.Update();

    Model bunnyModel(myEngine.GetModelConfig(0));
    bunnyModel.Create("resources", "bunny.obj", myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), 5);

    Sphere sphere(myEngine.GetModelConfig(1));
    sphere.Create(myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());


    CollisionManager collisionManager;


    Vector3 scale = {};
    Vector3 rotation = {};
    Vector3 translation = {};
    Matrix4x4 worldMat = {};

    rotation = { 0.0f,0.0f,0.0f };
    scale = { 2.5f,2.5f,2.5f };
    translation = { 0.0f,1.0f,0.0f };

    MSG msg{};

    float toCubeSpeed = 1.0f / 240.0f;

    Vector4 worldColor = { 0.0f,0.0f,0.0f,1.0f };

    Vector2 speed = { 2.0f,2.0f };
    size_t currentIndex = 0;
    Vector4 colors[4] = { {1.0f,0.0f,0.0f,1.0f}, {0.0f,1.0f,0.0f,1.0f}, {0.0f,0.0f,1.0f,1.0f}, {1.0f,0.0f,1.0f,1.0f} };
    float timer = 0.0f;


    Player player;
    player.Init(bunnyModel);

    std::list< Mirror*> mirrors;
    for (int i = 0; i < 7; ++i) {
        Mirror* mirror = new Mirror();

        Model* mirrorModel = new Model(myEngine.GetModelConfig(0));
        mirrorModel->Create("resources/mirror", "mirror.obj", myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), 4);

        mirror->Init(*mirrorModel);
        mirror->SetTranslate({ (i + 1) * 4.0f,0.0f,0.0f });
        mirrors.push_back(mirror);
    }

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

            ImGui::Begin("WorldMatrix");
            const char* items[] = { "SPHERE" };
            debugUI.WorldMatrixUpdate(scale, rotation, translation, items[0]);
            ImGui::End();

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
                sphere.GetMaterial()->lightType = light_current % 3;
                bunnyModel.GetMaterial()->lightType = light_current % 3;

                ImGui::End();
            }

            debugUI.SphereUpdate(sphere);
            debugUI.SpriteUpdate(playerSprite);

            debugUI.InputUpdate(*input);
            debugUI.Color(worldColor);

            debugUI.DebugMirror(mirrors);

#endif

            worldMat = MakeAffineMatrix(scale, rotation, translation);

            sphere.SetUVScale({ 1.0f,50.0f,1.0f });
            sphere.GetUVTransform().translate += { 0.0f, 1.0f / 60.0f, 0.0f };
            sphere.UpdateUV();
            sphere.SetColor(Lerp(colors[currentIndex], colors[(currentIndex + 1) % 4], t));

            bunnyModel.GetWaveData(0).amplitude = 1.0f / 16.0f;
            bunnyModel.GetWaveData(0).time += 1.0f / 60.0f;

            sphere.GetExpansionData().cube += toCubeSpeed;
            if (sphere.GetExpansionData().cube <= 0.0f || sphere.GetExpansionData().cube >= 1.0f) {
                toCubeSpeed *= -1.0f;
            }
            sprite.SetColor(Lerp(colors[currentIndex], colors[(currentIndex + 1) % 4], t));
            sprite.GetTranslateRef() += {speed.x, speed.y, 0.0f};

            if (sprite.GetTranslateRef().x > myEngine.GetWC().GetClientWidth() - sprite.GetSize().x || sprite.GetTranslateRef().x < 0.0f) {
                speed.x *= -1.0f;
            }

            if (sprite.GetTranslateRef().y > myEngine.GetWC().GetClientHeight() - sprite.GetSize().y || sprite.GetTranslateRef().y < 0.0f) {
                speed.y *= -1.0f;
            }

            sprite.Update();

            if (input->IsTriggerKey(DIK_SPACE)) {
                //音声再生
                sound.SoundPlay(soundData2, 0.5f);
            }

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
                Vector3 currentCamPos = camera.GetTranslate(); // カメラの現在位置
                Vector3 targetCamPos = player.GetWorldPosition();
                targetCamPos.z -= 20.0f; // プレイヤーの後ろに配置

                // 補間：現在位置 → 目標位置へ滑らかに移動（αは追従の速さ）
                float followSpeed = 0.1f; // 0.0〜1.0（小さいほどゆっくり）
                Vector3 smoothedPos = Lerp(currentCamPos, targetCamPos, followSpeed);

                camera.SetTarnslate(smoothedPos);
                camera.Update();
            }


            player.Update();
            if (player.IsHit()) {

                if (!isVoiceSound) {
                    isVoiceSound = true;

                }


            }

            for (Mirror* mirror : mirrors) {
                mirror->Update();
                if (mirror->IsDead()) {

                    isSound = true;

                }
            }

            mirrors.remove_if([](Mirror* mirror) {
                if (mirror->IsDead()) {

                    delete mirror;
                    return true;
                }
                return false;
                });


            if (isSound) {
                sound.SoundPlay(soundData1, 0.125f);
                isSound = false;
            }

            if (isVoiceSound) {
                sound.SoundPlay(voiceData[rand() % 3], 0.125f);
                isVoiceSound = false;
            }

            collisionManager.RegisterList(&player, mirrors);
            collisionManager.CheckAllCollisions();
            collisionManager.ClearList();

#pragma region //描画


            myEngine.PreCommandSet(worldColor);

            //グリッドの描画
            grid.Draw(srv2);

            for (Mirror* mirror : mirrors) {
                mirror->Draw(camera);
            }

            player.Draw(camera);

            sphere.PreDraw();
            sphere.Draw(worldMat, camera, srv3);

            sprite.PreDraw();
            sprite.Draw(srv);

            playerSprite.Draw(srv4);

            myEngine.PostCommandSet();
#pragma endregion

        }
    }

    for (Mirror* mirror : mirrors) {

        delete mirror;

    }

    mirrors.clear();

    myEngine.End();

    return 0;
}
