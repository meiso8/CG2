#include "MyEngine.h"

//void MyEngine::Initialize(int width, int height) {
//
//    std::ofstream logStream = logFile_.CreateLogFile();
//
//    window_.Create(width, height);
//    Log(logStream, "CreateWindowClass");
//    dxgiFactory_.Create();
//    Log(logStream, "CreateDXGIFactory");
//    gpu_.SettingGPU(dxgiFactory_.GetDigiFactory());
//    Log(logStream, "Set GPU");
//    device_ = gpu_.CreateD3D12Device();
//    Log("Complete create D3D12Device!!!\n");//初期化完了のログを出す
//
//#ifdef _DEBUG
//    debugError_.Create(device_);
//    Log(logStream, "CreateDebugError");
//#endif
//
//    commandQueue_.Create(device_);
//    Log(logStream, "CreateCommandQueue");
//    commandList_.Create(device_);
//    Log(logStream, "CreateCommandList");
//    swapChain_.Create(width, height, dxgiFactory_.GetDigiFactory(), commandQueue_.GetCommandQueue(), window_.GetHwnd());
//    Log(logStream, "CreateSwapChain");
//
//    //DescriptorHeapを生成する
//    rtvDescriptorHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
//    Log(logStream, "Create RTV DescriptorHeap");
//    //SRV　SRVやCBV用のDescriptorHeapは一旦ゲーム中に一つだけ
//   srvDescriptorHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
//    Log(logStream, "Create SRV DescriptorHeap");
//
//    //SwapChainからResourceを引っ張ってくる
//    swapChain_.GetBuffer(0, swapChainResources_[0]);
//    swapChain_.GetBuffer(1, swapChainResources_[1]);
//    Log(logStream, "Pull Resource from SwapChain");
//
//#pragma region//DescriptorSIze
//    //DescriptorSizeを取得しておく
//    const uint32_t descriptorSizeRTV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//    const uint32_t descriptorSizeSRV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//    const uint32_t descriptorSizeDSV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
//#pragma endregion
//
//    //RTVを作る
//    rtv_.Create(device_, swapChainResources_, rtvDescriptorHeap_, descriptorSizeRTV);
//    Log(logStream, "CreateRTV");
//
//    //FenceとEventを生成する
//    fence_.Create(device_);
//    Log(logStream, "CreateFence");
//    fenceEvent_.Create();
//    Log(logStream, "CreateEvent");
//
//
//#pragma region// DXCの初期化　dxcCompilerを初期化
//    //dxcCompilerを初期化
//    IDxcUtils* dxcUtils = nullptr;
//    IDxcCompiler3* dxcCompiler = nullptr;
//    HRESULT result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
//    assert(SUCCEEDED(result));
//    result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
//    assert(SUCCEEDED(result));
//
//    //現時点ではincludeはしないが、includeに対応するための設定を行っていく
//    IDxcIncludeHandler* includeHandler = nullptr;
//    result = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
//    assert(SUCCEEDED(result));
//
//    Log(logStream, "InitDxcCompiler");
//
//#pragma endregion
//
//#pragma region//RootSignatureを生成する
//
//    //具体的にShaderがどこかでデータを読めばいいのかの情報を取りまとめたもの
//
//    //rootSignature作成
//    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
//    descriptionRootSignature.Flags =
//        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
//
//    //DescriptorRange
//    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
//    descriptorRange[0].BaseShaderRegister = 0;//0から始める
//    descriptorRange[0].NumDescriptors = 1;//1つ
//    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRV
//    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//オフセット自動計算
//
//    //Smaplerの設定
//    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
//    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイナリフィルタ
//    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0-1の範囲外をリピート
//    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
//    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
//    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較せぬ
//    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipmapを使う
//    staticSamplers[0].ShaderRegister = 0;
//    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
//    descriptionRootSignature.pStaticSamplers = staticSamplers;
//    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);
//
//    //CBufferを利用することになったので、RootParameterに設定を追加する
//   /* RootParameter作成。PixelShaderのMaterialとVertexShaderのTransform*/
//
//    rootParameters_[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
//    rootParameters_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
//    rootParameters_[0].Descriptor.ShaderRegister = 0;//レジスタ番号0を使う
//    rootParameters_[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
//    rootParameters_[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexShaderで使う
//    rootParameters_[1].Descriptor.ShaderRegister = 0;//レジスタ番号0を使う
//    rootParameters_[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//Table
//    rootParameters_[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
//    rootParameters_[2].DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
//    rootParameters_[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//Tableで利用する数
//    rootParameters_[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
//    rootParameters_[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
//    rootParameters_[3].Descriptor.ShaderRegister = 1;//レジスタ番号1を使う
//    rootParameters_[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;//SRVを使う
//    rootParameters_[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexShaderで使う
//    rootParameters_[4].Descriptor.ShaderRegister = 0;//レジスタ番号0を使う
//    rootParameters_[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
//    rootParameters_[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexShaderで使う
//    rootParameters_[5].Descriptor.ShaderRegister = 1;//レジスタ番号1を使う
//
//    descriptionRootSignature.pParameters = rootParameters_;//ルートパラメータ配列へのポインタ
//    descriptionRootSignature.NumParameters = _countof(rootParameters_);//配列の長さ
//
//    //シリアライズしてバイナリにする
//    Microsoft::WRL::ComPtr <ID3DBlob> signatureBlob = nullptr;
//    Microsoft::WRL::ComPtr <ID3DBlob> errorBlob = nullptr;
//   HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
//        D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
//
//    if (FAILED(hr)) {
//        Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
//        assert(false);
//    }
//
//    //バイナリ元に生成
//    Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
//    hr = device_->CreateRootSignature(0,
//        signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
//        IID_PPV_ARGS(&rootSignature));
//    assert(SUCCEEDED(hr));
//
//    Log(logStream, "CreateRootSignature");
//
//#pragma endregion
//
//#pragma region//InputLayout
//
//    //InputLayout
//    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
//    inputElementDescs[0].SemanticName = "POSITION";
//    inputElementDescs[0].SemanticIndex = 0;
//    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//RGBA
//    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
//    inputElementDescs[1].SemanticName = "TEXCOORD";
//    inputElementDescs[1].SemanticIndex = 0;
//    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;//Vector2のためRG
//    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
//    inputElementDescs[2].SemanticName = "NORMAL";
//    inputElementDescs[2].SemanticIndex = 0;
//    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;//RGB
//    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
//
//    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
//    inputLayoutDesc.pInputElementDescs = inputElementDescs;
//    inputLayoutDesc.NumElements = _countof(inputElementDescs);
//
//    Log(logStream, "InputLayout");
//
//#pragma endregion
//
//    //BlendStateの設定を行う
//    BlendState blendState;
//    blendState.Create();
//    Log(logStream, "SetBlendState");
//
//    //RasterizerStateの設定を行う
//    RasterizerState rasterizerState;
//    rasterizerState.Create();
//    Log(logStream, "SetRasterizerState");
//
//#pragma region//ShaderをCompileする
//
////Shaderをコンパイルする
//    Microsoft::WRL::ComPtr <IDxcBlob> vertexShaderBlob = CompileShader(L"resources/shader/Object3D.VS.hlsl",
//        L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
//    assert(vertexShaderBlob != nullptr);
//    Log(logStream, "CompileVertexShader");
//
//    Microsoft::WRL::ComPtr <IDxcBlob>pixelShaderBlob = CompileShader(L"resources/shader/Object3D.PS.hlsl",
//        L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
//    assert(pixelShaderBlob != nullptr);
//    Log(logStream, "CompilePixelShader");
//
//#pragma endregion
//
//#pragma region //DepthStencilStateの設定
//    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
//    //Depthの機能を有効化する
//    depthStencilDesc.DepthEnable = true;
//    //書き込みします
//    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
//    //比較関数はLessEqual。つまり、近ければ描画される
//    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
//#pragma endregion
//
//    //PSOを生成する
//    PSO pso;
//    pso.Create(
//        rootSignature, inputLayoutDesc, vertexShaderBlob, pixelShaderBlob,
//        blendState.GetBlendDesc(), rasterizerState.GetRasterizerDesc(), depthStencilDesc, device_);
//    Log(logStream, "CreatePSO");
//
//}
//
//void MyEngine::BeginFrame() {
//    // バックバッファのインデックス取得やバリア設定など
//}
//
//void MyEngine::EndFrame() {
//    // Presentやフェンス処理など
//}
//
//void MyEngine::Finalize() {
//    // リソース解放など
//
//
//    CloseHandle(fenceEvent_.GetEvent());
//    CloseWindow(window_.GetHwnd());
//}