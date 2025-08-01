#include<numbers>
#include"MyEngine.h"

#include"FPSCounter.h"

#define WIN_WIDTH 1280
#define WIN_HEIGHT 720

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
    SoundData soundData1 = sound.SoundLoad(L"resources/Sounds/Alarm01.wav");
    SoundData soundData2 = sound.SoundLoad(L"resources/Sounds/maou_se_battle_explosion05.mp3");

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


    DrawGrid grid = DrawGrid(myEngine.GetDevice(), camera, myEngine.GetModelConfig(0));

    Sprite sprite;
    sprite.Create(myEngine.GetDevice(), cameraSprite, myEngine.GetModelConfig(1));
    sprite.SetSize(Vector2(256.0f, 128.0f));

    Model model(myEngine.GetModelConfig(0));
    model.Create("resources", "teapot.obj", myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), 3);

    Model bunnyModel(myEngine.GetModelConfig(0));
    bunnyModel.Create("resources", "bunny.obj", myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), 5);

    Model multiMesh(myEngine.GetModelConfig(2));
    multiMesh.Create("resources", "multiMesh.obj", myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap(), 6);

    Sphere sphere(myEngine.GetModelConfig(1));
    sphere.Create(myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());

    const int maxMatrix = 4;

    enum Matrix {
        TEAPOT,
        BUNNY,
        SPHERE,
        MULTIMESH,
        MAX_MATRIX
    };

    Vector3 scale[MAX_MATRIX] = {};
    Vector3 rotation[MAX_MATRIX] = {};
    Vector3 translation[MAX_MATRIX] = {};
    Matrix4x4 worldMat[MAX_MATRIX] = {};

    for (int i = 0; i < MAX_MATRIX; ++i) {
        scale[i] = { 1.0f,1.0f,1.0f };
        rotation[i] = { 0.0f,0.0f,0.0f };
        worldMat[i] = MakeAffineMatrix(scale[i], rotation[i], translation[i]);
    }

    scale[SPHERE] = { 2.5f,2.5f,2.5f };

    translation[TEAPOT] = { -7.5f,0.75f,0.0f };
    translation[BUNNY] = { 0.0f };
    translation[SPHERE] = { 0.0f,1.0f,0.0f };
    translation[MULTIMESH] = { -10.0f,1.0f,0.0f };

    MSG msg{};

    bool isExpansion = false;
    float expansionSpeed = 1.0f / 120.0f;
    float toCubeSpeed = 1.0f / 240.0f;

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
            const char* items[] = { "TEAPOT", "BUNNY", "SPHERE","MULTIMESH"};
            for (int i = 0; i < MAX_MATRIX; ++i) {
                debugUI.WorldMatrixUpdate(scale[i], rotation[i], translation[i], items[i]);
            }
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
                sphere.GetMaterial()->lightType = light_current;
                bunnyModel.GetMaterial()->lightType = light_current;
                model.GetMaterial()->lightType = light_current;
                multiMesh.GetMaterial()->lightType = light_current;

                ImGui::End();
            }

            debugUI.SphereUpdate(sphere);
            debugUI.SpriteUpdate(sprite);

            debugUI.ModelUpdate(model);
            debugUI.InputUpdate(input);

  
#endif

            rotation[TEAPOT].y += 1.0f / 60.0f;
            rotation[MULTIMESH].y += 1.0f / 60.0f;
            worldMat[TEAPOT] = Multiply(MakeAffineMatrix(scale[TEAPOT], rotation[TEAPOT], translation[TEAPOT]), MakeRotateYMatrix(rotation[TEAPOT].y));

            for (int i = BUNNY; i < MAX_MATRIX; ++i) {
                worldMat[i] = MakeAffineMatrix(scale[i], rotation[i], translation[i]);
            }

            sphere.SetUVScale({ 1.0f,50.0f,1.0f });
            sphere.GetUVTransform().translate += { 0.0f, 1.0f / 60.0f, 0.0f };
            sphere.UpdateUV();
            sphere.SetColor(Lerp(colors[currentIndex], colors[(currentIndex + 1) % 4], t));

            bunnyModel.GetWaveData(0).amplitude = 1.0f / 64.0f;
            bunnyModel.GetWaveData(0).time += 1.0f / 60.0f;

            model.UpdateUV();

            if (isExpansion) {
                bunnyModel.GetExpansionData().expansion += expansionSpeed;

                bool isSmall = bunnyModel.GetExpansionData().expansion <= 0.0f;

                if (isSmall) {
                    isExpansion = false;
                    bunnyModel.GetExpansionData().expansion = 0.0f;
                }

                if (bunnyModel.GetExpansionData().expansion >= 0.18f || isSmall) {
                    expansionSpeed *= -1.0f;

                }

            }

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

            //視点操作
            input.EyeOperation(camera);

            if (input.IsTriggerKey(DIK_1)) {
                //音声再生
                sound.SoundPlay(soundData1);
            }

            if (input.IsTriggerKey(DIK_SPACE)) {
                //音声再生
                sound.SoundPlay(soundData2);
                isExpansion = true;
            }

            if (input.IsTriggerKey(DIK_RETURN)) {
                debugCamera.SetIsOrthographic(true);
            }

            if (input.IsTriggerKey(DIK_D)) {
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
                //カメラの更新処理
                camera.Update();
            }

#pragma region //描画


            myEngine.PreCommandSet(worldColor);

            grid.Draw(srv2);

            model.PreDraw();

            model.Draw(worldMat[TEAPOT], camera);
            bunnyModel.Draw(worldMat[BUNNY], camera);
            multiMesh.PreDraw();
            multiMesh.Draw(worldMat[MULTIMESH], camera);

            sphere.PreDraw();
            sphere.Draw(worldMat[SPHERE], camera, srv3);

            sprite.PreDraw();
            sprite.Draw(srv);


            myEngine.PostCommandSet();
#pragma endregion

        }
    }

    myEngine.End();

    return 0;
}
