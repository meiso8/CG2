#include<numbers>
#include"MyEngine.h"

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

    D3DResourceLeakChecker leakCheck;
    //main関数の先頭でComの初期化を行う
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    assert(SUCCEEDED(hr));

    //誰も捕捉しなかった場合に(Unhandled),補足する関数を登録
    //main関数始まってすぐに登録すると良い
    SetUnhandledExceptionFilter(ExportDump);

    LogFile logFile;
    std::ofstream logStream = logFile.CreateLogFile();

    //WindowClassの生成
    Window wc;
    wc.Create(1280, 720);

    Log(logStream, "CreateWindowClass");

    //DXGIFactoryの生成
    DXGIFactory dxgiFactory;
    dxgiFactory.Create();
    Log(logStream, "CreateDXGIFactory");

    //使用するアダプタ(GPU)を決定する
    GPU gpu;
    gpu.SettingGPU(dxgiFactory.GetDigiFactory());
    Log(logStream, "Set GPU");

    //D3D12Deviceの生成
    Microsoft::WRL::ComPtr<ID3D12Device> device = CreateD3D12Device(gpu.GetUseAdapter());
    Log("Complete create D3D12Device!!!\n");//初期化完了のログを出す
    //ファイルへのログ出力
    Log(logStream, "Complete create D3D12Device!!!\n");


    //DirectInputオブジェクト
    Input input;
    input.Initalize(wc.GetWindowClass(), wc.GetHwnd());

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

#ifdef _DEBUG
    DebugError debugError;
    debugError.Create(device);
    Log(logStream, "SetDebugError");
#endif

#pragma endregion

    //コマンドキューの生成
    CommandQueue commandQueue;
    commandQueue.Create(device);
    Log(logStream, "CreateCommandQueue");

    //コマンドリストの生成
    CommandList commandList;
    commandList.Create(device);
    Log(logStream, "CreateCommandList");

    //スワップチェインの生成
    SwapChain swapChainClass;
    //画面の幅。ウィンドウのクライアント領域を同じものにしておく
    swapChainClass.Create(
        wc.GetClientWidth(),
        wc.GetClientHeight(),
        dxgiFactory.GetDigiFactory(),
        commandQueue.GetCommandQueue(),
        wc.GetHwnd());
    Log(logStream, "CreateSwapChain");

#pragma region//DescriptorHeapを生成する
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
    Log(logStream, "Create RTV DescriptorHeap");
#pragma endregion

#pragma region //SRV　SRVやCBV用のDescriptorHeapは一旦ゲーム中に一つだけ
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
    Log(logStream, "Create SRV DescriptorHeap");
#pragma endregion

#pragma region//SwapChainからResourceを引っ張ってくる
    //SwapChainからResourceを引っ張ってくる
    Microsoft::WRL::ComPtr <ID3D12Resource> swapChainResources[2] = { nullptr };

    swapChainClass.GetBuffer(0, swapChainResources[0]);
    swapChainClass.GetBuffer(1, swapChainResources[1]);
    Log(logStream, "Pull Resource from SwapChain");

#pragma endregion

#pragma region//DescriptorSIze
    //DescriptorSizeを取得しておく
    const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
#pragma endregion

    //RTVを作る
    RenderTargetView rtvClass;
    rtvClass.Create(device, swapChainResources, rtvDescriptorHeap, descriptorSizeRTV);
    Log(logStream, "CreateRTV");

#pragma region //FenceとEventを生成する

    Fence fence;
    fence.Create(device);

    // FenceのSignalを持つためのイベントを作成する
    FenceEvent fenceEventClass;
    fenceEventClass.Create();
    Log(logStream, "CreateFence&Event");

#pragma endregion

#pragma region// DXCの初期化　dxcCompilerを初期化

    DxcCompiler dxcCompiler;
    dxcCompiler.Initialize();
    Log(logStream, "InitDxcCompiler");

#pragma endregion

#pragma region//RootSignatureを生成する

    //具体的にShaderがどこかでデータを読めばいいのかの情報を取りまとめたもの

    //rootSignature作成
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    //DescriptorRange
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;//0から始める
    descriptorRange[0].NumDescriptors = 1;//1つ
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRV
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//オフセット自動計算

    //Smaplerの設定
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイナリフィルタ
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0-1の範囲外をリピート
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較せぬ
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipmapを使う
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

    //CBufferを利用することになったので、RootParameterに設定を追加する
   /* RootParameter作成。PixelShaderのMaterialとVertexShaderのTransform*/
    D3D12_ROOT_PARAMETER rootParameters[6] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
    rootParameters[0].Descriptor.ShaderRegister = 0;//レジスタ番号0を使う
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexShaderで使う
    rootParameters[1].Descriptor.ShaderRegister = 0;//レジスタ番号0を使う
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//Table
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//Tableで利用する数
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
    rootParameters[3].Descriptor.ShaderRegister = 1;//レジスタ番号1を使う
    rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;//SRVを使う
    rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexShaderで使う
    rootParameters[4].Descriptor.ShaderRegister = 0;//レジスタ番号0を使う
    rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
    rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexShaderで使う
    rootParameters[5].Descriptor.ShaderRegister = 1;//レジスタ番号1を使う

    descriptionRootSignature.pParameters = rootParameters;//ルートパラメータ配列へのポインタ
    descriptionRootSignature.NumParameters = _countof(rootParameters);//配列の長さ

    //シリアライズしてバイナリにする
    Microsoft::WRL::ComPtr <ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr <ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature,
        D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

    if (FAILED(hr)) {
        Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }

    //バイナリ元に生成
    Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
    hr = device->CreateRootSignature(0,
        signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));

    Log(logStream, "CreateRootSignature");

#pragma endregion

#pragma region//InputLayout

    //InputLayout

    InputLayout inputLayout;
    inputLayout.Create();

    Log(logStream, "InputLayout");

#pragma endregion

    //BlendStateの設定を行う
    BlendState blendState;
    blendState.Create();
    Log(logStream, "SetBlendState");

    //RasterizerStateの設定を行う
    RasterizerState rasterizerState;
    rasterizerState.Create();
    Log(logStream, "SetRasterizerState");

#pragma region//ShaderをCompileする

    dxcCompiler.ShaderSeting();
    Log(logStream, "CompileShader");

#pragma endregion

#pragma region //DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    //Depthの機能を有効化する
    depthStencilDesc.DepthEnable = true;
    //書き込みします
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    //比較関数はLessEqual。つまり、近ければ描画される
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
#pragma endregion

    //PSOを生成する
    PSO pso;
    pso.Create(
        rootSignature, inputLayout.GetDesc(), dxcCompiler.GetVertexShaderBlob(), dxcCompiler.GetPixelShaderBlob(),
        blendState.GetDesc(), rasterizerState.GetDesc(), depthStencilDesc, device);
    Log(logStream, "CreatePSO");

#pragma region //Texrureを読んで転送する

    DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = CreateTextureResource(device, metadata);
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = UploadTextureData(textureResource.Get(), mipImages, device, commandList.GetComandList());

#pragma endregion

#pragma endregion

    //ShaderResourceViewを作る
    ShaderResourceView srvClass = {};
    srvClass.Create(metadata, textureResource, 1, device, srvDescriptorHeap, descriptorSizeSRV);

#pragma region//Camera

    bool isDebug = false;

    DebugCamera debugCamera;
    debugCamera.Initialize(&input, static_cast<float>(wc.GetClientWidth()), static_cast<float>(wc.GetClientHeight()));

    Camera camera;
    Transform cameraTransform{ { 1.0f, 1.0f, 1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,-5.0f } };
    camera.SetTransform(cameraTransform);
    camera.Initialize(static_cast<float>(wc.GetClientWidth()), static_cast<float>(wc.GetClientHeight()), false);

    Camera cameraSprite;
    cameraSprite.Initialize(static_cast<float>(wc.GetClientWidth()), static_cast<float>(wc.GetClientHeight()), true);

#pragma endregion



#pragma region//time

    int waveCount = 2;

    Microsoft::WRL::ComPtr <ID3D12Resource> WaveResource = CreateBufferResource(device, sizeof(Wave) * waveCount);

    //データを書き込む
    Wave* waveData = nullptr;
    //書き込むためのアドレスを取得
    WaveResource->Map(0, nullptr, reinterpret_cast<void**>(&waveData));

    waveData[0].direction = { 1.0f,0.0f,0.0f };
    waveData[0].time = 0.0f;
    waveData[0].amplitude = 0.0f;
    waveData[0].frequency = 4;

    waveData[1].direction = { 1.0f,0.0f,0.0f };
    waveData[1].time = 0.0f;
    waveData[1].amplitude = 0.0f;
    waveData[1].frequency = 4;

#pragma endregion

#pragma region//Balloon

    Microsoft::WRL::ComPtr <ID3D12Resource> expansionResource = CreateBufferResource(device, sizeof(Balloon));

    //データを書き込む
    Balloon* expansionData = nullptr;
    //書き込むためのアドレスを取得
    expansionResource->Map(0, nullptr, reinterpret_cast<void**>(&expansionData));

    expansionData->expansion = 0.0f;
    expansionData->sphere = 0.0f;
    expansionData->cube = 0.0f;
    expansionData->isSphere = false;

#pragma endregion


#pragma region//stencileTextureResourceの作成
    Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilResource = CreateDepthStencileTextureResource(device, wc.GetClientWidth(), wc.GetClientHeight());

    //DSV用ヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
    Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

    //DSVの設定 DepthStencilView
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//基本的にはResourceに合わせる。
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2dTexture
    // DSVHeapの先頭にDSVを作る
    device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

#pragma endregion

#pragma region//平行光源用のResourceを作成する
    Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResource = CreateBufferResource(device, sizeof(DirectionalLight));

    //データを書き込む
    DirectionalLight* directionalLightData = nullptr;
    //書き込むためのアドレスを取得
    directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
    //デフォルト値はとりあえず以下のようにしておく   
    directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
    directionalLightData->direction = { 0.0f,-1.0f,0.0f };//向きは正規化する
    directionalLightData->intensity = 1.0f;

#pragma endregion

    //ViewportとScissor(シザー)
    D3D12_VIEWPORT viewport = CreateViewport(static_cast<float>(wc.GetClientWidth()), static_cast<float>(wc.GetClientHeight()));
    D3D12_RECT scissorRect = CreateScissorRect(wc.GetClientWidth(), wc.GetClientHeight());
    Log(logStream, "ViewportAndScissor");

#ifdef _DEBUG
    //ImGuiの初期化。
    ImGuiClass imGuiClass;
    imGuiClass.Initialize(wc.GetHwnd(), device.Get(), swapChainClass.GetSwapChainDesc(), rtvClass.GetDesc(), srvDescriptorHeap);
    Log(logStream, "InitImGui");
#endif

    Model model(camera, commandList, viewport, scissorRect, rootSignature, pso);
    model.Create("resources/cube", "cube.obj", device, srvDescriptorHeap, descriptorSizeSRV);

    MSG msg{};
    //ファイルへのログ出力
    Log(logStream, "LoopStart");

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

#ifdef _DEBUG
            //ImGuiにここからフレームが始まる旨を伝える
            imGuiClass.FrameStaert();
#endif

#pragma region //ゲームの処理

#ifdef _DEBUG

            ImGui::Begin("Wave1");
            ImGui::DragFloat("time1", &waveData[0].time, 0.03f);
            ImGui::DragFloat("amplitude1", &waveData[0].amplitude, 0.03f);
            ImGui::DragFloat3("direction1", &waveData[0].direction.x, 0.03f, 0.0f, 1.0f);
            ImGui::SliderFloat("frequency1", &waveData[0].frequency, 1.0f, 10.0f);
            ImGui::End();
            Vector3 waveDirection1 = waveData[0].direction;
            waveData[0].direction = Normalize(waveDirection1);

            ImGui::Begin("Wave2");
            ImGui::DragFloat("time2", &waveData[1].time, 0.03f);
            ImGui::DragFloat("amplitude2", &waveData[1].amplitude, 0.03f);
            ImGui::DragFloat3("direction2", &waveData[1].direction.x, 0.03f, 0.0f, 1.0f);
            ImGui::SliderFloat("frequency2", &waveData[1].frequency, 1.0f, 10.0f);
            ImGui::End();
            Vector3 waveDirection2 = waveData[1].direction;
            waveData[1].direction = Normalize(waveDirection2);

            ImGui::Begin("expansion");
            ImGui::DragFloat("expansionData", &expansionData->expansion, 0.03f);
            ImGui::DragFloat("sphere", &expansionData->sphere, 0.03f, 0.0f, 1.0f);
            ImGui::DragFloat("cube", &expansionData->cube, 0.03f, 0.0f, 1.0f);
            ImGui::Checkbox("isSphere", &expansionData->isSphere);

            ImGui::End();


#pragma region//Lightを設定

            Vector3 direction = directionalLightData->direction;

            ImGui::Begin("DirectionalLight");
            ImGui::DragFloat4("color", &directionalLightData->color.x);
            ImGui::SliderFloat3("direction", &direction.x, -1.0f, 1.0f);//後で正規化する
            directionalLightData->direction = Normalize(direction);

            ImGui::DragFloat("intensity", &directionalLightData->intensity);
            ImGui::End();
#pragma endregion

#pragma region//Modelのデバッグ 

            //開発用のUIの処理。実際に開発用のUIを出す場合はここkをゲーム固有の処理に置き換える
         /*   ImGui::ShowDemoWindow();*/
            ImGui::Begin("Model");
            ImGui::SliderFloat3("scale", &model.GetTransformRef().scale.x, 0.0f, 8.0f);
            ImGui::SliderFloat3("rotate", &model.GetTransformRef().rotate.x, 0.0f, std::numbers::pi_v<float>*2.0f);
            ImGui::SliderFloat3("translate", &model.GetTransformRef().translate.x, -2.0f, 2.0f);
            ImGui::ColorPicker4("materialColor", &(model.Getmaterial()->color.x));
            ImGui::DragFloat4("vertexData0", &(model.GetVertexData()[0].position.x));
            ImGui::DragFloat4("vertexData1", &(model.GetVertexData()[1].position.x));
            ImGui::DragFloat4("vertexData2", &(model.GetVertexData()[2].position.x));

            if (ImGui::Button("Init")) {

                model.InitTraslate();

            }

            ImGui::End();

#pragma endregion

#endif

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

            //Modelの更新
            model.Update();

#ifdef _DEBUG
            //ImGuiの内部コマンドを生成する
            imGuiClass.Render();
#endif

            //これからの流れ
            //1.  BackBufferを決定する
            //2.  書き込む作業（画面のクリア）をしたいので、RTVを設定する
            //3.  画面のクリアを行う
            //4.  CommandListを閉じる
            //5.  CommandListの実行（キック）
            //6.  画面のスワップ（BackBufferとFrontBufferを入れ替える）
            //7.  次のフレーム用にCommandListを再準備

            //1.これから書き込むバックバッファのインデックスを取得
            UINT backBufferIndex = swapChainClass.GetSwapChain()->GetCurrentBackBufferIndex();

            //TransitionBarrierの設定
            TransitionBarrier barrierClass;
            barrierClass.SettingBarrier(swapChainResources[backBufferIndex], commandList.GetComandList());

            //2.描画用のRTVとDSVを設定する
            D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            commandList.GetComandList()->OMSetRenderTargets(1, &rtvClass.GetHandle(backBufferIndex), false, &dsvHandle);
            //3.指定した色で画面全体をクリアする
            float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };//青っぽい色。RGBAの順
            commandList.GetComandList()->ClearRenderTargetView(rtvClass.GetHandle(backBufferIndex), clearColor, 0, nullptr);

            //指定した深度で画面全体をクリアする
            commandList.GetComandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


            //描画用のDescriptorHeapの設定
            ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap.Get() };
            commandList.GetComandList()->SetDescriptorHeaps(1, descriptorHeaps);

#pragma region //Modelを描画する

            Log(logStream, "DrawModel");
            model.PreDraw();
            model.Draw();

            //LightのCBufferの場所を設定
            commandList.GetComandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
            //timeのSRVの場所を設定
            commandList.GetComandList()->SetGraphicsRootShaderResourceView(4, WaveResource->GetGPUVirtualAddress());
            //expansionのCBufferの場所を設定
            commandList.GetComandList()->SetGraphicsRootConstantBufferView(5, expansionResource->GetGPUVirtualAddress());

            //DrawCall
            model.DrawCall();

#pragma endregion

#ifdef _DEBUG
            //諸々の描画処理が終了下タイミングでImGuiの描画コマンドを積む
            imGuiClass.DrawImGui(commandList);

#endif // _DEBUG


            //画面に書く処理は終わり、画面に移すので、状態を遷移

            barrierClass.Transition();

            //TransitionBarrierを張る
            commandList.GetComandList()->ResourceBarrier(1, &barrierClass.GetBarrier());

            //4.コマンドリストの内容を確定させる。全てのコマンドを詰んでから　Closesすること。
            hr = commandList.GetComandList()->Close();
            assert(SUCCEEDED(hr));
            Log(logStream, "CloseCommandList");

            //5.GPUにコマンドリストの実行を行わせる
            ID3D12CommandList* commandLists[] = { commandList.GetComandList().Get() };
            commandQueue.GetCommandQueue()->ExecuteCommandLists(1, commandLists);
            //6.GPUとOSに画面の交換を行うよう通知する
            swapChainClass.GetSwapChain()->Present(1, 0);

            //画面の更新が終わった直後GPUにシグナルを送る
            //Fenceの値を更新
            fence.AddValue();

            //GPUがここまでたどり着いた時、Fenceの値を指定した値に代入するようにSignalを送る
            fence.SendSignal(commandQueue.GetCommandQueue());

            //Fenceの値が指定したSignal値にたどり着いているか確認する GPUの処理を待つ
            fence.CheckValue(fenceEventClass.GetEvent());

            //7.次のフレーム用のコマンドリストを準備
            commandList.PrepareCommand();

#pragma endregion

        }
    }

#ifdef _DEBUG
    //ImGuiの終了処理 ゲームループが終わったら行う
    imGuiClass.ShutDown();
#endif

#pragma region //解放処理

    //音声データの解放
    sound.SoundUnload(&soundData1);
    sound.SoundUnload(&soundData2);

    CloseHandle(fenceEventClass.GetEvent());
    CloseWindow(wc.GetHwnd());

#pragma endregion

    CoUninitialize();

    return 0;
}
