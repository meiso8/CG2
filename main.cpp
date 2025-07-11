#include<numbers>
#include"MyEngine.h"
#include"Header/math/SphericalCoordinate.h"

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
    texture.Load("resources/uvChecker.png");

    //ShaderResourceViewを作る
    ShaderResourceView srv = {};
    srv.Create(texture, 1, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());

    Texture texture2 = Texture(myEngine.GetDevice(), myEngine.GetCommandList());
    texture2.Load("resources/white2x2.png");

    //ShaderResourceViewを作る
    ShaderResourceView srv2 = {};
    srv2.Create(texture2, 2, myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());

    Line line[102];

    for (int i = 0; i < 102; ++i) {
        line[i].Create(myEngine.GetDevice(), camera, myEngine.GetModelConfig());
    }

    for (int i = 0; i < 51; ++i) {
        line[i].SetTranslate({ 0.0f,0.0f, static_cast<float>(i - 25) });
    }

    for (int i = 0; i < 51; ++i) {
        line[i + 51].SetRotate(Vector3{ 0.0f,std::numbers::pi_v<float> / 2.0f,0.0f });
        line[i + 51].SetTranslate({ static_cast<float>(i - 25),0.0f, 0.0f });
    }

    line[25].SetColor(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
    line[25].SetScale(Vector3(1.0f, 10.0f, 10.0f));

    line[76].SetColor(Vector4(0.0f, 1.0f, 0.0f, 1.0f));
    line[76].SetScale(Vector3(1.0f, 10.0f, 10.0f));


    Sprite sprite;
    sprite.Create(myEngine.GetDevice(), cameraSprite, myEngine.GetModelConfig());

    Model model(myEngine.GetModelConfig());
    model.Create("resources/cube", "cube.obj", myEngine.GetDevice(), myEngine.GetSrvDescriptorHeap());

    Vector2 offset = { 0.0f,0.0f };
    Vector2 currentPos = { 0.0f };
    Vector2 delta = { 0.0f };
    Vector3 pos = { 0.0f };
    ShericalCoordinate sc = { 10.0f,0.0f,0.0f };

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
            input.InputInfoGet();

            //エンジンのアップデート
            myEngine.Update();

#pragma region //ゲームの処理

            ImGui::Begin("Input");
            ImGui::SliderFloat("polar", &sc.polar, -10.0f, 10.0f);
            ImGui::SliderFloat("azimuthal", &sc.azimuthal, -10.0f, 10.0f);
            ImGui::SliderFloat("radius", &sc.radius, -100.0f, 100.0f);
            ImGui::SliderFloat3("camera", &camera.GetRotate().x, -3.14f, 3.14f);
            ImGui::SliderFloat2("startPos", &offset.x, -100.0f, 100.0f);
            ImGui::SliderFloat2("currentPos", &currentPos.x, -100.0f, 100.0f);
            ImGui::End();

            ImGui::Begin("Line");
            ImGui::SliderFloat3("0scale", &line[0].GetScaleRef().x, -100.0f, 100.0f);
            ImGui::SliderFloat3("0rotate", &line[0].GetRotateRef().x, -100.0f, 100.0f);
            ImGui::SliderFloat3("0translate", &line[0].GetTranslateRef().x, -100.0f, 100.0f);
            ImGui::SliderFloat3("0vertexData0", &line[0].GetVertexData(0).position.x, -100.0f, 100.0f);
            ImGui::SliderFloat3("0vertexData1", &line[0].GetVertexData(1).position.x, -100.0f, 100.0f);
            ImGui::SliderFloat3("0vertexData2", &line[0].GetVertexData(2).position.x, -100.0f, 100.0f);
            ImGui::SliderFloat3("0vertexData3", &line[0].GetVertexData(3).position.x, -100.0f, 100.0f);

            ImGui::SliderFloat3("1scale", &line[1].GetScaleRef().x, -100.0f, 100.0f);
            ImGui::SliderFloat3("1rotate", &line[1].GetRotateRef().x, -100.0f, 100.0f);
            ImGui::SliderFloat3("1translate", &line[1].GetTranslateRef().x, -100.0f, 100.0f);

            ImGui::SliderFloat3("2scale", &line[2].GetScaleRef().x, -100.0f, 100.0f);
            ImGui::SliderFloat3("2rotate", &line[2].GetRotateRef().x, -100.0f, 100.0f);
            ImGui::SliderFloat3("2translate", &line[2].GetTranslateRef().x, -100.0f, 100.0f);

            ImGui::SliderFloat3("3scale", &line[3].GetScaleRef().x, -100.0f, 100.0f);
            ImGui::SliderFloat3("3rotate", &line[3].GetRotateRef().x, -100.0f, 100.0f);
            ImGui::SliderFloat3("3translate", &line[3].GetTranslateRef().x, -100.0f, 100.0f);

            ImGui::SliderFloat3("4scale", &line[4].GetScaleRef().x, -100.0f, 100.0f);
            ImGui::SliderFloat3("4rotate", &line[4].GetRotateRef().x, -100.0f, 100.0f);
            ImGui::SliderFloat3("4translate", &line[4].GetTranslateRef().x, -100.0f, 100.0f);

            ImGui::End();

#pragma region//視点操作
            if (input.IsPressMouse(2) && input.IsPushKey(DIK_LSHIFT)) {
                //視点の移動 offset をずらす
                //後でoffsetをくわえる
                offset += input.GetMousePos();
                camera.SetOffset({ offset.x / 120,offset.y / 60 });
            } else if (input.IsPressMouse(2)) {
                //視点の回転
                //中ボタン押し込み&&ドラッグ
                input.isDragging_ = true;
            }

            //マウススクロールする //初期位置-10
            sc.radius = -10 + input.GetMouseWheel();

            if (!input.IsPressMouse(2)) {
                input.isDragging_ = false;
            }

            if (input.isDragging_) {
                currentPos = input.GetMousePos();
                sc.polar += currentPos.x / 120;
                sc.azimuthal += currentPos.y / 120;
                camera.SetRotateY(sc.polar);
                camera.SetRotateZ(sc.azimuthal);
            }

            pos = TransformCoordinate(sc);

            camera.SetTarnslate(pos);

#pragma endregion

            if (input.IsTriggerKey(DIK_1)) {
                //音声再生
                sound.SoundPlay(soundData1);
            }

            if (input.IsTriggerKey(DIK_2)) {
                //音声データの解放
              /*  sound.SoundUnload(&soundData1);*/
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


            myEngine.PreCommandSet();


#pragma region //Modelを描画する

            line[0].PreDraw();

            for (int i = 0; i < 102; ++i) {
                line[i].Update();
                line[i].Draw(srv2);
            }

            model.PreDraw();
            model.Draw(MakeIdentity4x4(), camera);


#pragma endregion

            myEngine.PostCommandSet();

        }
    }

    myEngine.End();

    return 0;
}
