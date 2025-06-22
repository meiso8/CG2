#include<numbers>
#include<format>//フォーマットを推論してくれる

#pragma region //自作関数
#include"Header/Window.h"
#include"Header/CommandQueue.h"
#include"Header/CommandList.h"
#include"Header/DXGIFactory.h"
#include"Header/SwapChain.h"
#include"Header/DescriptorHeap.h"
#include"Header/RenderTargetView.h"
#include"Header/GetCPUDescriptorHandle.h"
#include"Header/GetGPUDescriptorHandle.h"
#include"Header/DebugError.h"
#include"Header/TransitionBarrier.h"
#include"Header/Fence.h"
#include"Header/FenceEvent.h"
#include"Header/D3DResourceLeakChecker.h"
#include"Header/Depth.h"//StencilTextureの作成関数　奥行き
#include"Header/CompileShader.h"
#include"Header/BlendState.h"
#include"Header/RasterizerState.h"
#include"Header/PSO.h"
#include"Header/ViewPort.h"
#include"Header/ScissorRect.h"
#include"Header/Texture.h"
#include"Header/CreateBufferResource.h"
#include"Header/ShaderResourceView.h"
#include"Header/Model.h"
#include"Header/Sprite.h"
#include"Header/Sound.h"
#include"Header/Input.h"
#include"Header/DebugCamera.h"
#include"Header/Camera.h"
#include"Header/CrashHandler.h"
#include"Header/Log.h"
#include"Header/ImGuiClass.h"

#include"Header/Material.h"
#include"Header/VertexData.h"
#include"Header/DirectionalLight.h"
#include"Header/TransformationMatrix.h"
#include"Header/ModelData.h"

#include"Header/math/Normalize.h"
#include"Header/math/Transform.h"
#include"Header/math/MakeIdentity4x4.h"
#include"Header/math/MakeAffineMatrix.h"
#include"Header/math/Inverse.h"
#include"Header/math/MakePerspectiveFovMatrix.h"
#include"Header/math/MakeOrthographicMatrix.h"
#include"Header/math/Multiply.h"

#include"Header/Wave.h"//波打ちアニメーション用
#include"Header/Balloon.h"

#pragma endregion

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

    D3DResourceLeakChecker leakCheck;
    //main関数の先頭でComの初期化を行う
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    assert(SUCCEEDED(hr));

    //誰も捕捉しなかった場合に(Unhandled),補足する関数を登録
    //main関数始まってすぐに登録すると良い
    SetUnhandledExceptionFilter(ExportDump);

    //　出力ウィンドウへの文字入力
    OutputDebugStringA("Hello,DirectX!\n");

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

#pragma region//使用するアダプタ(GPU)を決定する

    Microsoft::WRL::ComPtr <IDXGIAdapter4> useAdapter = nullptr;
    //良い順にアダプタを頼む
    for (UINT i = 0;   dxgiFactory.GetDigiFactory()->EnumAdapterByGpuPreference(i,
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

    //ファイルへのログ出力
    Log(logStream, "Set GPU");

#pragma endregion

#pragma region//D3D12Deviceの生成

    Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;

    //機能レベル(FEATURE_LEVEL)とログの出力用の文字列
    D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
    };

    const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
    //高い順に生成出来るか試していく
    for (size_t i = 0; i < _countof(featureLevels); ++i) {
        //採用したアダプターでデバイスを生成
        hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
        //指定した機能レベルでデバイスが生成できたかを確認する
        if (SUCCEEDED(hr)) {

            Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
            break;
        }
    }

    //デバイスの生成が上手くいかなかったので起動できない
    assert(device != nullptr);
    Log("Complete create D3D12Device!!!\n");//初期化完了のログを出す

    //ファイルへのログ出力
    Log(logStream, "Complete create D3D12Device!!!\n");

#pragma endregion

    //DirectInputオブジェクト
    Input input;
    input.Initalize(wc.GetWindowClass(), wc.GetHwnd());

#pragma region//XAudio全体の初期化と音声の読み込み
    //DirectX初期化処理の末尾に追加する
    //音声クラスの作成
    Sound sound;
    hr = sound.Initialize();
    assert(SUCCEEDED(hr));

    //ここはゲームによって異なる
     //音声読み込み SoundDataの変数を増やせばメモリが許す限りいくつでも読み込める。
    SoundData soundData1 = sound.SoundLoad(L"resources/Alarm01.wav");
    SoundData soundData2 = sound.SoundLoad(L"resources/dreamcore.mp3");

#pragma endregion

#ifdef _DEBUG

    DebugError debugError;
    debugError.Create(device);
    //ファイルへのログ出力をしました
    Log(logStream, "SetDebugError");

#endif

#pragma endregion

    //コマンドキューの生成
    CommandQueue commandQueue;
    commandQueue.Create(device);
    //ファイルへのログ出力
    Log(logStream, "CreateCommandQueue");

    //コマンドリストの生成
    CommandList commandList;
    commandList.Create(device);
    //ファイルへのログ出力
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
    //ファイルへのログ出力
    Log(logStream, "CreateSwapChain");


#pragma region//DescriptorHeapを生成する

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
    //ファイルへのログ出力
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

    //ファイルへのログ出力
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
    //ファイルへのログ出力
    Log(logStream, "CreateRTV");

#pragma region //FenceとEventを生成する

    Fence fence;
    fence.Create(device);

    // FenceのSignalを持つためのイベントを作成する
    FenceEvent fenceEventClass;
    fenceEventClass.Create();

    //ファイルへのログ出力
    Log(logStream, "CreateFence&Event");

#pragma endregion

#pragma region// DXCの初期化　dxcCompilerを初期化
    //dxcCompilerを初期化
    IDxcUtils* dxcUtils = nullptr;
    IDxcCompiler3* dxcCompiler = nullptr;
    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    assert(SUCCEEDED(hr));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
    assert(SUCCEEDED(hr));

    //現時点ではincludeはしないが、includeに対応するための設定を行っていく
    IDxcIncludeHandler* includeHandler = nullptr;
    hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
    assert(SUCCEEDED(hr));

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
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//RGBA
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;//Vector2のためRG
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[2].SemanticName = "NORMAL";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;//RGB
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

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

    //Shaderをコンパイルする
    Microsoft::WRL::ComPtr <IDxcBlob> vertexShaderBlob = CompileShader(L"resources/shader/Object3D.VS.hlsl",
        L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(vertexShaderBlob != nullptr);

    Log(logStream, "CompileVertexShader");

    Microsoft::WRL::ComPtr <IDxcBlob>pixelShaderBlob = CompileShader(L"resources/shader/Object3D.PS.hlsl",
        L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(pixelShaderBlob != nullptr);

    Log(logStream, "CompilePixelShader");

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

    //PSO
#pragma region//PSOを生成する

    PSO pso;
    pso.Create(
        rootSignature, inputLayoutDesc, vertexShaderBlob, pixelShaderBlob,
        blendState.GetBlendDesc(), rasterizerState.GetRasterizerDesc(), depthStencilDesc, device);

    Log(logStream, "CreatePSO");

#pragma endregion

#pragma region //Texrureを読んで転送する

    DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = CreateTextureResource(device, metadata);
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = UploadTextureData(textureResource.Get(), mipImages, device, commandList.GetComandList());

#pragma endregion

#pragma region//Resourceにデータを書き込む

    //モデルの読み込み
    ModelData modelData = LoadObjeFile("resources/cube", "cube.obj");
    //頂点リソースを作る
    Microsoft::WRL::ComPtr<ID3D12Resource>vertexResource = CreateBufferResource(device, sizeof(VertexData) * modelData.vertices.size());
    Log(logStream, "CreateVertexResource");

    //頂点バッファビューを作成する
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();//リソースの先頭アドレスから使う
    vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());//使用するリソースのサイズは頂点のサイズ
    vertexBufferView.StrideInBytes = sizeof(VertexData);//1頂点あたりのサイズ
    Log(logStream, "CreateVertexBufferView");

    //頂点リソースにデータを書き込む
    VertexData* vertexData = nullptr;
    vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));//書き込むためのアドレスを取得
    std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());//頂点データをリソースにコピー
    Log(logStream, "WriteDateToResource");

    //モデルのテクスチャを読む
    DirectX::ScratchImage mipImages2 = LoadTexture(modelData.material.textureFilePath);
    const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
    Microsoft::WRL::ComPtr<ID3D12Resource>textureResource2 = CreateTextureResource(device, metadata2);
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource2 = UploadTextureData(textureResource2.Get(), mipImages2, device, commandList.GetComandList());

#pragma endregion

#pragma region ShaderResourceViewを作る

    ShaderResourceView srvClass[2] = {};

    srvClass[0].Create(metadata, textureResource, 1, device, srvDescriptorHeap, descriptorSizeSRV);
    srvClass[1].Create(metadata2, textureResource2, 2, device, srvDescriptorHeap, descriptorSizeSRV);

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

#pragma region//Camera

    DebugCamera debugCamera;

    debugCamera.Initialize(&input, static_cast<float>(wc.GetClientWidth()), static_cast<float>(wc.GetClientHeight()));

    bool isDebug = false;

    Camera camera;
    Transform cameraTransform{ { 1.0f, 1.0f, 1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,-5.0f } };
    camera.SetTransform(cameraTransform);
    camera.Initialize(static_cast<float>(wc.GetClientWidth()), static_cast<float>(wc.GetClientHeight()), false);

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

#pragma region//ViewportとScissor(シザー)

    D3D12_VIEWPORT viewport = CreateViewport(static_cast<float>(wc.GetClientWidth()), static_cast<float>(wc.GetClientHeight()));
    D3D12_RECT scissorRect = CreateScissorRect(wc.GetClientWidth(), wc.GetClientHeight());
    Log(logStream, "ViewportAndScissor");

#pragma endregion

#pragma region//ImGuiの初期化。
#ifdef _DEBUG

    ImGuiClass imGuiClass;
    imGuiClass.Initialize(wc.GetHwnd(), device.Get(), swapChainClass.GetSwapChainDesc(), rtvClass.GetDesc(), srvDescriptorHeap);
    Log(logStream, "InitImGui");
#endif
#pragma endregion

    Model model;
    model.Create(device, camera);

    Camera cameraSprite;
    cameraSprite.Initialize(static_cast<float>(wc.GetClientWidth()), static_cast<float>(wc.GetClientHeight()), true);

    Sprite sprite;
    sprite.Create(device, cameraSprite);

    bool uvCheck = false;

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
            ImGui::Checkbox("useMonsterBall", &uvCheck);
            ImGui::SliderFloat3("scale", &model.GetTransformRef().scale.x, 0.0f, 8.0f);
            ImGui::SliderFloat3("rotate", &model.GetTransformRef().rotate.x, 0.0f, std::numbers::pi_v<float>*2.0f);
            ImGui::SliderFloat3("translate", &model.GetTransformRef().translate.x, -2.0f, 2.0f);
            ImGui::ColorPicker4("materialColor", &(model.Getmaterial()->color.x));
            ImGui::DragFloat4("vertexData0", &(vertexData[0].position.x));
            ImGui::DragFloat4("vertexData1", &(vertexData[1].position.x));
            ImGui::DragFloat4("vertexData2", &(vertexData[2].position.x));

            if (ImGui::Button("Init")) {

                model.InitTraslate();

            }

            ImGui::End();

#pragma endregion

#pragma region//Spriteのデバッグ

            ImGui::Begin("Sprite");

            ImGui::SliderFloat3("scale", &sprite.GetTranslateRef().x, 0.0f, 4.0f);
            ImGui::SliderFloat3("rotate", &sprite.GetRotateRef().x, 0.0f, std::numbers::pi_v<float>*2.0f);
            ImGui::SliderFloat3("translate", &sprite.GetTranslateRef().x, -128.0f, 1280.0f);
            ImGui::ColorPicker4("materialColor", &sprite.Getmaterial()->color.x);
            ImGui::DragFloat2("uv : scale", &sprite.GetUVTranslateRef().x, 0.01f, -10.0f, 10.0f);
            ImGui::DragFloat2("uv : rotate", &sprite.GetUVRotateRef().x, 0.01f, -10.0f, 10.0f);
            ImGui::SliderAngle("uv : translate", &sprite.GetUVTranslateRef().x);

            ImGui::End();

#pragma endregion

#endif

            if (input.IsTriggerKey(DIK_1)) {
                //音声再生
                sound.SoundPlay(soundData1);
                uvCheck = (uvCheck) ? false : true;
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
                camera.SetprojectionMatrix(debugCamera.GetProjectionMatrix());
                debugCamera.Update();

            } else {
                //カメラの更新処理
                camera.Update();
            }

            //Modelの更新
            model.Update();

            //Spriteの更新処理
            sprite.Update();

#ifdef _DEBUG
            //ImGuiの内部コマンドを生成する
  /*          ImGui::Render();*/
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


            commandList.GetComandList()->RSSetViewports(1, &viewport);//Viewportを設定
            commandList.GetComandList()->RSSetScissorRects(1, &scissorRect);//Scirssorを設定
            //RootSignatureを設定。PSOに設定しているけど別途設定が必要
            commandList.GetComandList()->SetGraphicsRootSignature(rootSignature.Get());
            commandList.GetComandList()->SetPipelineState(pso.GetGraphicsPipelineState().Get());//PSOを設定
            //形状を設定。PSOに設定している物とはまた別。同じものを設定すると考えておけばよい。
            commandList.GetComandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            //ファイルへのログ出力
            Log(logStream, "DrawModel");

            model.Draw(commandList, vertexBufferView, srvClass, uvCheck);

            //LightのCBufferの場所を設定
            commandList.GetComandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
            //timeのSRVの場所を設定
            commandList.GetComandList()->SetGraphicsRootShaderResourceView(4, WaveResource->GetGPUVirtualAddress());
            //LightのCBufferの場所を設定
            commandList.GetComandList()->SetGraphicsRootConstantBufferView(5, expansionResource->GetGPUVirtualAddress());

            //DrawCall
            model.DrawCall(commandList, modelData);

#pragma endregion

            //IndexSpriteの描画
            sprite.Draw(commandList, srvClass[0]);

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

    CoUninitialize();

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

    return 0;
}
