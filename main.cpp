#include <Windows.h>
#include<cstdint>//int32_tを使うため
#include<string>//ログの文字列を出力するため
#include<format>//フォーマットを推論してくれる
#include<d3d12.h>
#include<dxgi1_6.h>
#include<cassert> //assertも利用するため
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
//libのリンクはヘッダに書いてはいけない
//任意のひとつのcppに記述するかプロジェクトの設定で行う
//libのリンク includeのすぐ後ろに書くとよい
#include <dxgidebug.h>//リソースリークチェックのため
#pragma comment(lib,"dxguid.lib")

//ログを出力する関数
void Log(const std::string& message) {
    OutputDebugStringA(message.c_str());
}

//std::stringの基本的な使い方

////文字列を格納する
//std::string str0{ "STRING!!!" };
//
////整数を文字列にする
//std::string str1{ std::to_string(10) };

// https://cpprefjp.github.io/reference/string/basic_string.html

//string->wstringに変換する関数
std::wstring ConvertString(const std::string& str) {
    if (str.empty()) {
        return std::wstring();
    }

    auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
    if (sizeNeeded == 0) {
        return std::wstring();
    }
    std::wstring result(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
    return result;
}

//wstring->stringに変換する関数
std::string ConvertString(const std::wstring& str) {
    if (str.empty()) {
        return std::string();
    }

    auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
    if (sizeNeeded == 0) {
        return std::string();
    }
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
    return result;
}

// ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    //メッセージに応じてゲーム固有の処理を行う
    switch (msg) {
        //ウィンドウが破棄された
    case WM_DESTROY:
        //OSに対して、アプリの終了を伝える
        PostQuitMessage(0);
        return 0;
    }

    //標準メッセージ処理を行う
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

    //　出力ウィンドウへの文字入力
    OutputDebugStringA("Hello,DirectX!\n");

#pragma region ウィンドウクラスの登録

    WNDCLASS wc{};
    //ウィンドウプロシージャ
    wc.lpfnWndProc = WindowProc;
    //ウィンドウクラス名
    wc.lpszClassName = L"CG2WindowClass";
    //インスタンスハンドル
    wc.hInstance = GetModuleHandle(nullptr);
    //カーソル
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    //ウィンドウクラス名を登録する
    RegisterClass(&wc);

#pragma endregion

#pragma region ウィンドウサイズの設定

    //クライアント領域のサイズ
    const int32_t kClientWidth = 1280;
    const int32_t kClientHeight = 720;

    //ウィンドウサイズを表す構造体にクライアント領域を入れる
    RECT wrc = { 0,0,kClientWidth,kClientHeight };

    //クライアント領域をもとに実際のサイズにwrcを変更してもらう
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

#pragma endregion

#pragma region ウィンドウの生成と表示

    //ウィンドウの生成
    //CreateWindowの戻り値であるHWNDはウィンドウハンドルを呼びウィンドウを表す識別子である
    HWND hwnd = CreateWindow(
        wc.lpszClassName,         // 利用するクラス名
        L"CG2",                   // タイトルバーの文字(何でもいい)
        WS_OVERLAPPEDWINDOW,      // よく見るウィンドウスタイル
        CW_USEDEFAULT,            // 表示X座標(Windowsに任せる)
        CW_USEDEFAULT,            // 表示Y座標(WindowsOsに任せる)
        wrc.right - wrc.left,     // ウィンドウ横幅
        wrc.bottom - wrc.top,      // ウィンドウ縦幅
        nullptr,                  // 親ウィンドウハンドル
        nullptr,                  // メニューハンドル
        wc.hInstance,             // インスタンスハンドル
        nullptr);                 // オプション

#ifdef _DEBUG
    ID3D12Debug1* debugController = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        //デバッグレイヤーを有効化する
        debugController->EnableDebugLayer();
        //さらにGPU側でもチェックを行うようにする
        debugController->SetEnableGPUBasedValidation(TRUE);
    }
#endif

    //ウィンドウを表示する
    ShowWindow(hwnd, SW_SHOW);
#pragma endregion 

#pragma region//DXGIFactoryの生成

    //DXGIファクトリーの生成
    IDXGIFactory7* dxgiFactory = nullptr;
    //HRESULTはWindow系のエラーコードであり、
    //関数が成功したかどうかをSUCCEEDEDマクロで判定できる
    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
    //初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が多いのでassertにしておく
    assert(SUCCEEDED(hr));

#pragma endregion

#pragma region//使用するアダプタ(GPU)を決定する

    IDXGIAdapter4* useAdapter = nullptr;
    //良い順にアダプタを頼む
    for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
        DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
        DXGI_ERROR_NOT_FOUND; ++i) {
        //アダプタの情報を取得する
        DXGI_ADAPTER_DESC3 adapterDesc{};
        hr = useAdapter->GetDesc3(&adapterDesc);
        assert(SUCCEEDED(hr));//取得できないのは一大事

        //ソフトウェアアダプタでなければ採用！
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
            //採用したアダプタ情報をログに出力。wstringの方なので注意
            Log(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
            break;
        }

        useAdapter = nullptr;//ソフトウェアアダプタの場合は見なかったことにする

    }

    //適切なアダプタが見つからなかったので起動できない
    assert(useAdapter != nullptr);

#pragma endregion

#pragma region//D3D12Deviceの生成

    ID3D12Device* device = nullptr;
    //機能レベル(FEATURE_LEVEL)とログの出力用の文字列
    D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
    };

    const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
    //高い順に生成出来るか試していく
    for (size_t i = 0; i < _countof(featureLevels); ++i) {
        //採用したアダプターでデバイスを生成
        hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
        //指定した機能レベルでデバイスが生成できたかを確認する
        if (SUCCEEDED(hr)) {

            Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
            break;
        }
    }

    //デバイスの生成が上手くいかなかったので起動できない
    assert(device != nullptr);
    Log("Complete create D3D12Device!!!\n");//初期化完了のログを出す

#pragma endregion

#pragma region//エラーや警告時のデバッグ

    //デバイスに対してデバッグ
    ID3D12InfoQueue* infoQueue = nullptr;
    if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
        //ヤバいエラー時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        //エラー時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        //警告時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

        //解放を忘れたことが判明した場合、警告で停止する設定を外すことで、詳細な情報をログに出力することが出来る。
        //上記をコメントアウトし、情報を得て修正が終わったら必ず元に戻し停止しないことを確認する。

#pragma region//エラーと警告の抑制

        //抑制するメッセージのID
        D3D12_MESSAGE_ID denyIds[] = {
            //Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
                //https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
                D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
        };

        //抑制するレベル
        D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
        D3D12_INFO_QUEUE_FILTER filter{};
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;
        filter.DenyList.NumSeverities = _countof(severities);
        filter.DenyList.pSeverityList = severities;
        //指定したメッセージの表示を抑制する
        infoQueue->PushStorageFilter(&filter);

#pragma endregion

        //解放
        infoQueue->Release();
    }

#pragma endregion

#pragma region//コマンドキューの生成

    //コマンドキューを生成する まとまった命令群を送るため
    ID3D12CommandQueue* commandQueue = nullptr;
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));

    //コマンドキューの生成が上手くいかなかったので起動できない
    assert(SUCCEEDED(hr));

#pragma endregion

#pragma region //コマンドリストの生成

    //コマンドリストに必要な命令保存用メモリ管理構造を生成

    //コマンドアロケータを生成する(Allocate 割り当てる)
    ID3D12CommandAllocator* commandAllocator = nullptr;
    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    //コマンドアロケータの生成が上手くいかなかったので起動できない
    assert(SUCCEEDED(hr));

    //コマンドリストを生成する
    ID3D12GraphicsCommandList* commandList = nullptr;
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr,
        IID_PPV_ARGS(&commandList));
    //コマンドリスト生成が上手くいかなかったので起動できない
    assert(SUCCEEDED(hr));

#pragma endregion

#pragma region//スワップチェインの生成
    IDXGISwapChain4* swapChain = nullptr;
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = kClientWidth;   //画面の幅。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc.Height = kClientHeight;//画面の高さ。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
    swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲットとして利用する
    swapChainDesc.BufferCount = 2;//ダブルバッファ
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタにうつしたら、中身を破棄
    //コマンドキュー、ウィンドウハンドル、設定を渡して生成する
    hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain));
    assert(SUCCEEDED(hr));

#pragma endregion

#pragma region//DescriptorHeapを生成する

    //ディスクリプタヒープを生成する
    ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//連打ーターゲットビュー用
    rtvDescriptorHeapDesc.NumDescriptors = 2;//ダブルバッフ用に2つ。多くても別に構わない
    hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
    //ディスクリプタヒープが作れなかったので起動できない
    assert(SUCCEEDED(hr));

#pragma endregion

#pragma region//SwapChainからResourceを引っ張ってくる
    //SwapChainからResourceを引っ張ってくる
    ID3D12Resource* swapChainResources[2] = { nullptr };
    hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
    //上手く取得出来なければ起動できない
    assert(SUCCEEDED(hr));
    hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
    assert(SUCCEEDED(hr));
    //必要なものが作れないような場合、実際の製品であれば、メッセージを表示するなどする必要がある
   //ただ学生のうちはassertで止めてしまえばいい　ここら辺が失敗するPCというのは対象にしていないし、企業も使っていない

#pragma endregion

#pragma region//RTVを作る

    //Descriptorは必ずDescriptorHandleというポインタのようなものを介して扱う必要がある
    //Viewを作るときは、どこのDescriptorに情報を格納するかを明示的に指定する必要がる

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGBに変換して書き込む
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2dテクスチャとして書き込む
    //ディスクリプタの先頭を取得する
    D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    // GetCPUDescriptorHandleForHeapStart();DescriptorHeapが管理する最初のDescriptorHandleを返す

    //RTVを2つ作るのでディスクリプタを2つ用意
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
    //まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
    rtvHandles[0] = rtvStartHandle;
    device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);
    //2つ目のディスクリプタハンドルを得る（自力で）
    rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    //GetDescriptorHandleIncrementSize() Descriptorのサイズは、最適化のため、GPUまたはドライバによって異なることが許可されている
    //なのでDirectX12に問い合わせて実際のサイズを取得する　このサイズはゲーム中に変化することはないので初期化時に取得しておけばよい

    //2つ目を作る
    device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);

#pragma endregion

#pragma region //FenceとEventを生成する
    //初期値0でFenceを作る
    ID3D12Fence* fence = nullptr;
    uint64_t fenceValue = 0;
    hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    assert(SUCCEEDED(hr));

    // FenceのSignalを持つためのイベントを作成する
    HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent != nullptr);
#pragma endregion

    MSG msg{};
    //ウィンドウのxボタンが押されるまでループ
    while (msg.message != WM_QUIT) {
        //Windowにメッセージが来ていたら最優先で処理させる
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            //ゲームの処理

#pragma region //ゲームの処理

            //これからの流れ
            //1.  BackBufferを決定する
            //2.  書き込む作業（画面のクリア）をしたいので、RTVを設定する
            //3.  画面のクリアを行う
            //4.  CommandListを閉じる
            //5.  CommandListの実行（キック）
            //6.  画面のスワップ（BackBufferとFrontBufferを入れ替える）
            //7.  次のフレーム用にCommandListを再準備

            //1.これから書き込むバックバッファのインデックスを取得
            UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

            //TransitionBarrierの設定
            D3D12_RESOURCE_BARRIER barrier{};
            //今回はバリアはTransition
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            //Noneにしておく
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            //バリアを張る対象のリソース。現在のバックバッファに対して行う
            barrier.Transition.pResource = swapChainResources[backBufferIndex];
            //遷移前（現在）のResourceState
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            //遷移後のResourceState
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            //TransitionBarrierを張る
            commandList->ResourceBarrier(1, &barrier);

            //2.描画用のRTVを設定する
            commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
            //3.指定した色で画面全体をクリアする
            float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };//青っぽい色。RGBAの順
            commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

            //画面に書く処理は終わり、画面に移すので、状態を遷移
            //今回はRenderTargetからPresentにする
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            //　TransitionBarrierを張る
            commandList->ResourceBarrier(1, &barrier);

            //4.コマンドリストの内容を確定させる。全てのコマンドを詰んでから　Closesすること。
            hr = commandList->Close();
            assert(SUCCEEDED(hr));

            //5.GPUにコマンドリストの実行を行わせる
            ID3D12CommandList* commandLists[] = { commandList };
            commandQueue->ExecuteCommandLists(1, commandLists);
            //6.GPUとOSに画面の交換を行うよう通知する
            swapChain->Present(1, 0);

            //画面の更新が終わった直後GPUにシグナルを送る
            //Fenceの値を更新
            fenceValue++;
            //GPUがここまでたどり着いた時、Fenceの値を指定した値に代入するようにSignalを送る
            commandQueue->Signal(fence, fenceValue);

            //Fenceの値が指定したSignal値にたどり着いているか確認する
            //GetCompletedValueの初期値はFence作成時に渡した初期値
            if (fence->GetCompletedValue() < fenceValue) {
                //指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
                fence->SetEventOnCompletion(fenceValue, fenceEvent);
                //イベントを待つ
                WaitForSingleObject(fenceEvent, INFINITE);
            }

            //7.次のフレーム用のコマンドリストを準備
            hr = commandAllocator->Reset();
            assert(SUCCEEDED(hr));
            hr = commandList->Reset(commandAllocator, nullptr);
            assert(SUCCEEDED(hr));

#pragma endregion

#pragma region//ログの出力

            //Log("GameLoop\n");

            //int enemyHp = 100;
            //char texturePath[] = "enemy.png";

            //formatは変数から型を推論してくれる
            //Log(std::format("enemyHp:{}, texturePath:{}\n", enemyHp, texturePath));
            ////https://cpprefjp.github.io/reference/format/format.html

            //DirectXから受け取る情報をログに出力する
            //Log(ConvertString(std::format(L"WSTRING{}\n", wstringValue)));

#pragma endregion

        }

    }

#pragma region //解放処理

    CloseHandle(fenceEvent);
    fence->Release();
    rtvDescriptorHeap->Release();
    swapChainResources[0]->Release();
    swapChainResources[1]->Release();
    swapChain->Release();
    commandList->Release();
    commandAllocator->Release();
    commandQueue->Release();
    device->Release();
    useAdapter->Release();
    dxgiFactory->Release();

#ifdef _DEBUG
    debugController->Release();
#endif

    CloseWindow(hwnd);

#pragma endregion

#pragma region //リソースリークチェック

    //リソースリークチェック
    IDXGIDebug1* debug;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
        debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
        debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
        debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
        debug->Release();
    };



#pragma endregion

    return 0;
}


