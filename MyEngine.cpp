#include "MyEngine.h"

void MyEngine::Create(int32_t clientWidth, int32_t clientHeight) {

    clientWidth_ = clientWidth;
    clientHeight_ = clientHeight;

    //main関数の先頭でComの初期化を行う
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    assert(SUCCEEDED(hr));

    //誰も捕捉しなかった場合に(Unhandled),補足する関数を登録
    //main関数始まってすぐに登録すると良い
    SetUnhandledExceptionFilter(ExportDump);

    logStream = logFile.CreateLogFile();

    //WindowClassの生成
    wc.Create(clientWidth, clientHeight);
    Log(logStream, "CreateWindowClass");

    //DXGIFactoryの生成
    dxgiFactory.Create();
    Log(logStream, "CreateDXGIFactory");

    //使用するアダプタ(GPU)を決定する
    gpu.SettingGPU(dxgiFactory);
    Log(logStream, "Set GPU");

    //D3D12Deviceの生成
    device = CreateD3D12Device(gpu);
    Log("Complete create D3D12Device!!!\n");//初期化完了のログを出す
    //ファイルへのログ出力
    Log(logStream, "Complete create D3D12Device!!!\n");

#ifdef _DEBUG
    debugError.Create(device);
    Log(logStream, "SetDebugError");
#endif

    //コマンドキューの生成
    commandQueue.Create(device);
    Log(logStream, "CreateCommandQueue");

    //コマンドリストの生成
    commandList.Create(device);
    Log(logStream, "CreateCommandList");

    //スワップチェインの生成
    //画面の幅。ウィンドウのクライアント領域を同じものにしておく
    swapChainClass.Create(
        wc,
        dxgiFactory.GetDigiFactory(),
        commandQueue.GetCommandQueue());
    Log(logStream, "CreateSwapChain");

#pragma region//DescriptorHeapを生成する
    rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
    Log(logStream, "Create RTV DescriptorHeap");
#pragma endregion

#pragma region //SRV　SRVやCBV用のDescriptorHeapは一旦ゲーム中に一つだけ
    srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
    Log(logStream, "Create SRV DescriptorHeap");
#pragma endregion

#pragma region//SwapChainからResourceを引っ張ってくる
    //SwapChainからResourceを引っ張ってくる
    swapChainClass.GetBuffer(0, swapChainResources[0]);
    swapChainClass.GetBuffer(1, swapChainResources[1]);
    Log(logStream, "Pull Resource from SwapChain");

#pragma endregion

#pragma region//DescriptorSIze
    //DescriptorSizeを取得しておく
    //const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
#pragma endregion

    //RTVを作る
    rtvClass.Create(device, swapChainResources, rtvDescriptorHeap);
    Log(logStream, "CreateRTV");

#pragma region //FenceとEventを生成する

    fence.Create(device);

    // FenceのSignalを持つためのイベントを作成する
    fenceEvent.Create();
    Log(logStream, "CreateFence&Event");
#pragma endregion

#pragma region// DXCの初期化　dxcCompilerを初期化
    dxcCompiler.Initialize();
    Log(logStream, "InitDxcCompiler");
#pragma endregion

#pragma region//RootSignatureを生成する

    //具体的にShaderがどこかでデータを読めばいいのかの情報を取りまとめたもの
    rootSignature.Create(device);
    Log(logStream, "CreateRootSignature");

#pragma endregion

#pragma region//InputLayout
    inputLayout.Create();
    Log(logStream, "InputLayout");
#pragma endregion

    //BlendStateの設定を行う
    blendState.Create(false);
    Log(logStream, "SetBlendState");

    //RasterizerStateの設定を行う
    rasterizerState.Create();
    Log(logStream, "SetRasterizerState");

#pragma region//ShaderをCompileする
    dxcCompiler.ShaderSeting();
    Log(logStream, "CompileShader");
#pragma endregion

#pragma region //DepthStencilStateの設定
    depthStencil.Create();
    Log(logStream, "Create depthStencilDesc");
#pragma endregion

    //PSOを生成する
    pso.Create(
        rootSignature,
        inputLayout,
        dxcCompiler,
        blendState,
        rasterizerState,
        depthStencil,
        device);
    Log(logStream, "CreatePSO");

#pragma region//time

    int waveCount = 2;

    waveResource = CreateBufferResource(device, sizeof(Wave) * waveCount);

    //データを書き込む

    //書き込むためのアドレスを取得
    waveResource->Map(0, nullptr, reinterpret_cast<void**>(&waveData));

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

    expansionResource = CreateBufferResource(device, sizeof(Balloon));

    //書き込むためのアドレスを取得
    expansionResource->Map(0, nullptr, reinterpret_cast<void**>(&expansionData));
    //データを書き込む
    expansionData->expansion = 0.0f;
    expansionData->sphere = 0.0f;
    expansionData->cube = 0.0f;
    expansionData->isSphere = false;

#pragma endregion

#pragma region//stencileTextureResourceの作成
    depthStencilResource = CreateDepthStencileTextureResource(device, clientWidth_, clientHeight_);

    //DSV用ヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
    dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

    //DSVの設定 DepthStencilView
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//基本的にはResourceに合わせる。
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2dTexture
    // DSVHeapの先頭にDSVを作る
    device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

#pragma endregion

#pragma region//平行光源用のResourceを作成する
    directionalLightResource = CreateBufferResource(device, sizeof(DirectionalLight));

    //書き込むためのアドレスを取得
    directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
    //デフォルト値はとりあえず以下のようにしておく   
    directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
    directionalLightData->direction = { 0.0f,-1.0f,0.0f };//向きは正規化する
    directionalLightData->intensity = 1.0f;

#pragma endregion

    //ViewportとScissor(シザー)
    viewport = CreateViewport(static_cast<float>(wc.GetClientWidth()), static_cast<float>(wc.GetClientHeight()));
    scissorRect = CreateScissorRect(wc.GetClientWidth(), wc.GetClientHeight());
    Log(logStream, "ViewportAndScissor");

    modelConfig_ = {
        &commandList,
        &viewport,
        &scissorRect,
        &rootSignature,
        &pso,
        directionalLightResource,
        waveResource,
        expansionResource
    };

#ifdef _DEBUG
    //ImGuiの初期化。
    imGuiClass.Initialize(wc, device.Get(), swapChainClass, rtvClass, srvDescriptorHeap);
    Log(logStream, "InitImGui");
#endif

    //ファイルへのログ出力
    Log(logStream, "LoopStart");
}

void MyEngine::Update() {


#ifdef _DEBUG

    //ImGuiにここからフレームが始まる旨を伝える
    imGuiClass.FrameStart();
#endif


#ifdef _DEBUG


#pragma region//Lightを設定
    Vector3 direction = directionalLightData->direction;

    ImGui::Begin("DirectionalLight");
    ImGui::DragFloat4("color", &directionalLightData->color.x);
    ImGui::SliderFloat3("direction", &direction.x, -1.0f, 1.0f);//後で正規化する
    directionalLightData->direction = Normalize(direction);

    ImGui::DragFloat("intensity", &directionalLightData->intensity);
    ImGui::End();
#pragma endregion

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

#endif

}

void MyEngine::PreCommandSet() {
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

    barrier.SettingBarrier(swapChainResources[backBufferIndex], commandList.GetComandList());

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

};

void MyEngine::PostCommandSet() {

#ifdef _DEBUG
    //諸々の描画処理が終了下タイミングでImGuiの描画コマンドを積む
    imGuiClass.DrawImGui(commandList);

#endif // _DEBUG

    //画面に書く処理は終わり、画面に移すので、状態を遷移
    barrier.Transition();

    //TransitionBarrierを張る
    commandList.GetComandList()->ResourceBarrier(1, &barrier.GetBarrier());

    //4.コマンドリストの内容を確定させる。全てのコマンドを詰んでから　Closesすること。
    HRESULT hr = commandList.GetComandList()->Close();
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
    fence.SendSignal(commandQueue);

    //Fenceの値が指定したSignal値にたどり着いているか確認する GPUの処理を待つ
    fence.CheckValue(fenceEvent);

    //7.次のフレーム用のコマンドリストを準備
    commandList.PrepareCommand();

#pragma endregion

};

void MyEngine::End() {

#ifdef _DEBUG
    //ImGuiの終了処理 ゲームループが終わったら行う
    imGuiClass.ShutDown();
#endif

#pragma region //解放処理

    CloseHandle(fenceEvent.GetEvent());
    CloseWindow(wc.GetHwnd());

#pragma endregion

    CoUninitialize();
}