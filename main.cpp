#include <Windows.h>
#include<cstdint>//int32_tを使うため
#include<string>//ログの文字列を出力するため

//ファイルの書いたり読んだりするライブラリ　音声の読み込みにも使用する
#include <fstream>
//istringstreamのためにインクルードする
#include<sstream>
//時刻を扱うライブラリ
#include <chrono>
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
//Debug用のあれこれを使えるようにする
#include <dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")
//StringCchPrintfWの利用のため
#include<strsafe.h>

//ファイルやディレクトリに関する操作を行うライブラリ
#include <filesystem>

//HLSLをGPUが解釈できる形にするためのインクルード
#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

//Textureデータを読み込むためにDirectXTex.hをインクルード
#include"externals/DirectXTex/DirectXTex.h"
//Textureの転送のために
#include"externals/DirectXTex/d3dx12.h"
//動的配列を扱うための標準ライブラリ
#include <vector>

//ComPtr(コムポインタ)
#include<wrl.h>

#pragma region //ImGuiのincludeと関数の外部宣言
#ifdef _DEBUG

#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
#pragma endregion

#pragma region //自作関数
#include"Header/Material.h"
#include"Header/VertexData.h"
#include"Header/ModelData.h"
#include"Header/TransformationMatrix.h"
#include"Header/Normalize.h"
#include"Header/DirectionalLight.h"
#include"Header/Transform.h"
#include "Header/MakeIdentity4x4.h"
#include"Header/MakeAffineMatrix.h"
#include"Header/Inverse.h"
#include"Header/MakePerspectiveFovMatrix.h"
#include"Header/MakeOrthographicMatrix.h"
#include"Header/Multiply.h"
#include"Header/ResourceObject.h"
#include"Header/Sound.h"
#include"Header/Input.h"
#include"Header/DebugCamera.h"
#pragma endregion

//ログを出力する関数
void Log(const std::string& message) {
    OutputDebugStringA(message.c_str());
}

void Log(std::ostream& os, const std::string& message) {
    os << message << std::endl;
    OutputDebugStringA(message.c_str());
}

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

#ifdef _DEBUG
    //ImGuiにメッセージを渡す
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
        return true;
    }
#endif

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

//CrashHandler
static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
    //時刻を取得して、時刻を名前に入れたファイルを作成。Dumpsディレクトリ以下に出力
    SYSTEMTIME time;
    GetLocalTime(&time);
    wchar_t filePath[MAX_PATH] = { 0 };
    CreateDirectory(L"./Dumps", nullptr);
    StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
    HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
    //processId(このexeのId)とクラッシュ(例外)の発生したthreadIdを取得
    DWORD processId = GetCurrentProcessId();
    DWORD threadId = GetCurrentThreadId();
    //設定情報を入力
    MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
    minidumpInformation.ThreadId = threadId;
    minidumpInformation.ExceptionPointers = exception;
    minidumpInformation.ClientPointers = TRUE;
    // Dumpを出力。MiniDumpNormalは最低限の情報を出力するフラグ
    MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInformation, nullptr, nullptr);
    //他に関連付けられているSEH例外ハンドラがあれば実行。通常はプロセスを終了する
    return EXCEPTION_EXECUTE_HANDLER;
}

//CompileShader関数
IDxcBlob* CompileShader(
    //CompilerするShaderファイルへのパス
    const std::wstring& filePath,
    //Compilerに使用するProfile
    const wchar_t* profile,
    //初期化で生成されたものを3つ
    IDxcUtils* dxcUtils,
    IDxcCompiler3* dxcCompiler,
    IDxcIncludeHandler* includeHandler) {
    // ここの中身をこの後書いていく
    // 1.hlslファイルを読み込む

#pragma region //1.hlslファイルを読む
//ここからシェーダーをコンパイルする旨をログに出す
    Log(ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));
    //hlslファイルを読む
    IDxcBlobEncoding* shaderSource = nullptr;
    HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
    //読めなかったら止める
    assert(SUCCEEDED(hr));
    //読み込んだファイルの内容を設定する
    DxcBuffer shaderSourceBuffer;
    shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
    shaderSourceBuffer.Size = shaderSource->GetBufferSize();
    shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTF8の文字コードであることを追加

#pragma endregion

    // 2.Compileする
#pragma region//2.Compileする

    LPCWSTR arguments[] = {
    filePath.c_str(),//コンパイル対象のhlslファイル名
    L"-E",L"main",//エントリーポイントの設定。基本的にはmain以外にはしない
    L"-T",profile,//ShaderProfileの設定
    L"-Zi",L"-Qembed_debug",//デバック用の情報を埋め込む
    L"-Od",//最適化を外しておく
    L"-Zpr",//メモリレイアウトは行優先
    };

    //実際にShaderをコンパイルする
    IDxcResult* shaderResult = nullptr;
    hr = dxcCompiler->Compile(
        &shaderSourceBuffer,//読み込んだファイル
        arguments,//コンパイルオプション
        _countof(arguments),//コンパイルオプションの数
        includeHandler,//includeが含まれた諸々
        IID_PPV_ARGS(&shaderResult)//コンパイル結果
    );


    //コンパイルエラーではなくdxcが起動できないなどの致命的な状況
    assert(SUCCEEDED(hr));


#pragma endregion

    // 3.警告・エラーが出ていないか確認する
#pragma region //警告・エラーが出ていないか確認する
//警告・エラーが出ていたらログに出して止める
    IDxcBlobUtf8* shaderError = nullptr;
    shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);

    if (shaderError != nullptr && shaderError->GetStringLength() != 0) {

        Log(shaderError->GetStringPointer());

        //警告・エラーダメ絶対
        assert(false);
    } else {
        Log("No shader compilation errors detected.");
    }
#pragma endregion

    // 4.Compile結果を受け取って返す
#pragma region//Compile結果を受け取って返す
    //コンパイル結果空実行用のバイナリ部分を取得 Blob = BinaryLargeObjectの略
    IDxcBlob* shaderBlob = nullptr;
    hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
    assert(SUCCEEDED(hr));
    //成功したログを出す
    Log(ConvertString(std::format(L"Compile Succeeded,path:{}profile;{}\n", filePath, profile)));
    //もう使わないリソース解放
    shaderSource->Release();
    shaderResult->Release();
    //実行用のバイナリを返却
    return shaderBlob;
#pragma endregion

}

//Resource作成の関数化
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(
    const Microsoft::WRL::ComPtr<ID3D12Device>& device,
    size_t sizeInBytes) {

    Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;

    //ヒープの設定
    D3D12_HEAP_PROPERTIES uploadHeapProperties{};
    uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//UploadHeapを使う
    //頂点リソースの設定
    D3D12_RESOURCE_DESC resourceDesc{};
    //バッファリソース。テクスチャの場合はまた別の設定をする
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = sizeInBytes;//リソースサイズ。
    //バッファの場合はこれらは1にする決まり
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    //バッファの場合はこれにする決まり
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    if (SUCCEEDED(device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
        &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&resource)))) {
        return resource;
    }

    return nullptr;

};

//DescriptorHeapの作成関数
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
    const Microsoft::WRL::ComPtr<ID3D12Device>& device,
    D3D12_DESCRIPTOR_HEAP_TYPE heapType,
    UINT numDescriptors,
    bool shaderVisible) {

    //ディスクリプタヒープを生成する
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
    descriptorHeapDesc.Type = heapType;//連打ーターゲットビュー用
    descriptorHeapDesc.NumDescriptors = numDescriptors;
    descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
    //ディスクリプタヒープが作れなかったので起動できない
    assert(SUCCEEDED(hr));
    return descriptorHeap;
}

//インデックスのDescriptorHandleを取得する関数
D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {

    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (descriptorSize * index);
    return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {

    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (descriptorSize * index);
    return handleGPU;
};

#pragma region//深度の関数
//StencilTextureの作成関数　奥行き
Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencileTextureResource(
    const Microsoft::WRL::ComPtr<ID3D12Device>& device,
    int32_t width,
    int32_t height) {
    //生成するResourceの設定
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width = width;//Textureの幅
    resourceDesc.Height = height;//高さ
    resourceDesc.MipLevels = 1;//mipmapの数
    resourceDesc.DepthOrArraySize = 1;//奥行き　or 配列Textureの配列数
    resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//DepthStencilとして利用可能なフォーマット
    resourceDesc.SampleDesc.Count = 1;//サンプリングカウント
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2次元
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//DepthStencilとして使う通知

    //利用するHeapの設定
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//VRAM上

    //深度値クリア設定
    D3D12_CLEAR_VALUE depthClearValue{};
    depthClearValue.DepthStencil.Depth = 1.0f;//1.0f(最大値)でクリア
    depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//フォーマット。Resourceと合わせる

    //Resourceの生成
    Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,//Heapの設定
        D3D12_HEAP_FLAG_NONE,//Heapの特殊な設定。特になし。
        &resourceDesc,//Resourceの設定
        D3D12_RESOURCE_STATE_DEPTH_WRITE,//深度値を書き込む状態にしておく
        &depthClearValue,//Clear最適地
        IID_PPV_ARGS(&resource));
    assert(SUCCEEDED(hr));

    return resource;
}

#pragma endregion

#pragma region //Textureの関数
//テクスチャの読み込み関数
DirectX::ScratchImage LoadTexture(const std::string& filePath) {

    //テクスチャファイルを読んでプログラムで扱えるようにする
    DirectX::ScratchImage image{};
    std::wstring filePathW = ConvertString(filePath);
    //sRBG空間で作られた物として読む。
    HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
    assert(SUCCEEDED(hr));

    //ミニマップの作成
    DirectX::ScratchImage mipImages{};
    hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
    assert(SUCCEEDED(hr));

    //ミニマップ付きのデータを返す
    return mipImages;
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(
    const Microsoft::WRL::ComPtr<ID3D12Device>& device,
    const DirectX::TexMetadata& metadata) {

    //1. metadataを基にResourceの設定
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width = UINT(metadata.width);//Textureの値
    resourceDesc.Height = UINT(metadata.height);//Textureの高さ
    resourceDesc.MipLevels = UINT16(metadata.mipLevels);//mipmapの数
    resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);//奥行き　or 配列Textureの配列数
    resourceDesc.Format = metadata.format;//TextureのFormat
    resourceDesc.SampleDesc.Count = 1;//サンプリングカウント。1固定。
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);//Textureの次元数。

    //2.利用するHeapの設定
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//GPUで処理するために書き換え
    //heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//WriteBackポリシーとは
    //heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//プロセッサの近くに配置

    //3.Resourceを生成する
    Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,//Heapの設定
        D3D12_HEAP_FLAG_NONE,//Heapの特殊な設定。特になし
        &resourceDesc,//Resourceの設定
        D3D12_RESOURCE_STATE_COPY_DEST,// データ転送される設定
        nullptr,//Clear最適地。使わない
        IID_PPV_ARGS(&resource));//ポインタのポインタ
    assert(SUCCEEDED(hr));
    return resource;

}

//TextureResourceにデータを転送する　GPU
[[nodiscard]]
Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(
    const Microsoft::WRL::ComPtr<ID3D12Resource>& texture,
    const DirectX::ScratchImage& mipImages,
    const Microsoft::WRL::ComPtr<ID3D12Device>& device,
    const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList) {
    std::vector<D3D12_SUBRESOURCE_DATA>subresources;
    DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
    uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresources.size()));
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(device, intermediateSize);//中間リソース
    UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, UINT(subresources.size()), subresources.data());
    //Textureへの転送後は利用できるよう,D3D12_RESOURCE_STATE_COPY_DESTからRESOURCE_STATE_GENERIC_READへResourceStateを変更する
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = texture.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;//これに変更
    commandList->ResourceBarrier(1, &barrier);
    return intermediateResource;

}

#pragma endregion

#pragma region//ModelDataの読み込み関数

ModelData LoadObjeFile(const std::string& directoryPath, const std::string& filename) {

    //1.中で必要となる変数の宣言
    ModelData modelData;
    std::vector<Vector4> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;
    std::string line;//ファイルから読み込んだ1行を格納するもの

    //2.ファイルを開く
    std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
    assert(file.is_open());//開けなかったら止める

    //3.実際にファイルを読み、ModelDataを構成していく
    //std::getlineはstreamから1行読んでstd::stringに格納する関数
    while (std::getline(file, line)) {
        std::string identifier;
        //文字列を分解しながら読むためのクラス（空白を区切り文字として読むことが可能）
        std::istringstream s(line); //これに対して>>すると空文字区切りで1ずつ値を読むことが出来る
        s >> identifier;//先頭の識別子を読む　IDを読む

        //IDに応じた処理
        if (identifier == "v") {
            Vector4 position;
            s >> position.x >> position.y >> position.z;
            position.y *= -1.0f;//座標系の統一のため
            position.w = 1.0f;//同次座標のため
            positions.push_back(position);

        } else if (identifier == "vt") {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;
            texcoords.push_back(texcoord);

        } else if (identifier == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;
            normal.y *= -1.0f;//座標系の統一のため
            normals.push_back(normal);
        } else if (identifier == "f") {
            //三角形を作る

            //面は三角形限定。その他は未対応
            for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
                std::string vertexDefinition;
                s >> vertexDefinition;
                //頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
                std::istringstream v(vertexDefinition);
                uint32_t elementIndices[3];
                for (int32_t element = 0; element < 3; ++element) {
                    std::string index;
                    std::getline(v, index, '/');//区切りでインデックスを読んでいく
                    elementIndices[element] = std::stoi(index);
                }

                //要素へのIndexから、実際の要素の値を取得して、頂点を構成する
                Vector4 position = positions[elementIndices[0] - 1];//1始まりなので-1する
                Vector2 texcoord = texcoords[elementIndices[1] - 1];
                Vector3 normal = normals[elementIndices[2] - 1];

                //まずobj通りに保存、格納する際に逆にする　座標系の統一のため
                VertexData triangle = { position,texcoord,normal };
                modelData.vertices.push_back(triangle);
            }

            //////頂点を逆順で登録することで、回り順を逆順にする
            //modelData.vertices.push_back(triangle[0]);
            //modelData.vertices.push_back(triangle[1]);
            //modelData.vertices.push_back(triangle[2]);

        }

    }

    //4.ModelDataを返す
    return modelData;

}

#pragma endregion

/// @brief リークチェックの構造体
struct D3DResourceLeakChecker {
    ~D3DResourceLeakChecker() {
        //リソースリークチェック
        IDXGIDebug1* debug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
            debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
            debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
            debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
            debug->Release();
        }
    }
};

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

#pragma region //ログをファイルに書き出す
    // ログのディレクトリを用意
    std::filesystem::create_directory("logs");
    //現在時刻を取得（UTC時刻）
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    //ログファイルの名前にコンマ何秒はいらないので、削って秒にする
    std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
        nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    //日本時刻(PCの設定時間)に変換
    std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };
    //formatを使ってファイル名を決定
    std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
    //時刻を使ってファイル名を出力
    std::string logFilePath = std::string("logs/") + dateString + "log";
    //ファイルを作って書き込み準備
    std::ofstream logStream(logFilePath);
#pragma endregion

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

    //ファイルへのログ出力
    Log(logStream, "registrationWindowClass");

#pragma endregion

#pragma region ウィンドウサイズの設定

    //クライアント領域のサイズ
    const int32_t kClientWidth = 1280;
    const int32_t kClientHeight = 720;

    //ウィンドウサイズを表す構造体にクライアント領域を入れる
    RECT wrc = { 0,0,kClientWidth,kClientHeight };

    //クライアント領域をもとに実際のサイズにwrcを変更してもらう
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

    //ファイルへのログ出力
    Log(logStream, "setWindowSize");

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


    //ファイルへのログ出力
    Log(logStream, "create&displayWindow");

#ifdef _DEBUG
    Microsoft::WRL::ComPtr <ID3D12Debug1> debugController = nullptr;
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
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
    //HRESULTはWindow系のエラーコードであり、
    //関数が成功したかどうかをSUCCEEDEDマクロで判定できる
    hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
    //初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が多いのでassertにしておく
    assert(SUCCEEDED(hr));

    //ファイルへのログ出力
    Log(logStream, "createDXGIFactory");

#pragma endregion

#pragma region//使用するアダプタ(GPU)を決定する

    Microsoft::WRL::ComPtr <IDXGIAdapter4> useAdapter = nullptr;
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

#pragma region//DirectInputオブジェクト

    Input input;
    input.Initalize(wc, hwnd);

#pragma endregion

#pragma region//DebugCamera

    DebugCamera debugCamera;

    debugCamera.Initialize(&input);

#pragma endregion

#pragma region//XAudio全体の初期化と音声の読み込み
    //DirectX初期化処理の末尾に追加する
    //音声クラスの作成
    Sound sound;
    hr = sound.Initialize();
    assert(SUCCEEDED(hr));
    hr = sound.InitializeMF();
    assert(SUCCEEDED(hr));

    //ここはゲームによって異なる
     //音声読み込み SoundDataの変数を増やせばメモリが許す限りいくつでも読み込める。
    SoundData soundData1 = sound.SoundLoadWave("resources/Alarm01.wav");
    SoundDataMP3 soundData2 = sound.SoundLoadMP3(L"resources/dreamcore.mp3");
    //std::string path = "resources/dreamcore.mp3";
    //SoundDataMP3 soundData2 = sound.SoundLoadMP3(ConvertString(path));

#pragma endregion

#pragma region//エラーや警告時のデバッグ

    //デバイスに対してデバッグ
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
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

    //ファイルへのログ出力をしました
    Log(logStream, "SetDebugError");

#pragma endregion

#pragma region//コマンドキューの生成

    //コマンドキューを生成する まとまった命令群を送るため
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));

    //コマンドキューの生成が上手くいかなかったので起動できない
    assert(SUCCEEDED(hr));

    //ファイルへのログ出力
    Log(logStream, "CreateCommandQueue");

#pragma endregion

#pragma region //コマンドリストの生成

    //コマンドリストに必要な命令保存用メモリ管理構造を生成

    //コマンドアロケータを生成する(Allocate 割り当てる)
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    //コマンドアロケータの生成が上手くいかなかったので起動できない
    assert(SUCCEEDED(hr));

    //コマンドリストを生成する
    Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList = nullptr;
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr,
        IID_PPV_ARGS(&commandList));
    //コマンドリスト生成が上手くいかなかったので起動できない
    assert(SUCCEEDED(hr));

    //ファイルへのログ出力
    Log(logStream, "CreateCommandList");

#pragma endregion

#pragma region//スワップチェインの生成
    Microsoft::WRL::ComPtr <IDXGISwapChain4> swapChain = nullptr;
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = kClientWidth;   //画面の幅。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc.Height = kClientHeight;//画面の高さ。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
    swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲットとして利用する
    swapChainDesc.BufferCount = 2;//ダブルバッファ
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタにうつしたら、中身を破棄
    //コマンドキュー、ウィンドウハンドル、設定を渡して生成する
    hr = dxgiFactory->CreateSwapChainForHwnd(
        commandQueue.Get(),
        hwnd, &swapChainDesc,
        nullptr, nullptr,
        reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
    assert(SUCCEEDED(hr));

    //ファイルへのログ出力
    Log(logStream, "CreateSwapChain");

#pragma endregion

#pragma region//DescriptorHeapを生成する

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

    //ファイルへのログ出力
    Log(logStream, "CreateRTVDescriptorHeap");



#pragma endregion

#pragma region //SRV　SRVやCBV用のDescriptorHeapは一旦ゲーム中に一つだけ

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

#pragma endregion

#pragma region//SwapChainからResourceを引っ張ってくる
    //SwapChainからResourceを引っ張ってくる
    Microsoft::WRL::ComPtr <ID3D12Resource> swapChainResources[2] = { nullptr };
    hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
    //上手く取得出来なければ起動できない
    assert(SUCCEEDED(hr));
    hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
    assert(SUCCEEDED(hr));
    //必要なものが作れないような場合、実際の製品であれば、メッセージを表示するなどする必要がある
   //ただ学生のうちはassertで止めてしまえばいい　ここら辺が失敗するPCというのは対象にしていないし、企業も使っていない

    //ファイルへのログ出力
    Log(logStream, "Pull Resource from SwapChain");

#pragma endregion

#pragma region//DescriptorSIze
    //DescriptorSizeを取得しておく
    const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
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
    device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
    //2つ目のディスクリプタハンドルを得る（自力で）

    rtvHandles[1] = GetCPUDescriptorHandle(rtvDescriptorHeap.Get(), descriptorSizeRTV, 1);

    //GetDescriptorHandleIncrementSize() Descriptorのサイズは、最適化のため、GPUまたはドライバによって異なることが許可されている
    //なのでDirectX12に問い合わせて実際のサイズを取得する　このサイズはゲーム中に変化することはないので初期化時に取得しておけばよい

    //2つ目を作る
    device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);

    //ファイルへのログ出力
    Log(logStream, "CreateRTV");

#pragma endregion

#pragma region //FenceとEventを生成する
    //初期値0でFenceを作る
    Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
    uint64_t fenceValue = 0;
    hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    assert(SUCCEEDED(hr));

    // FenceのSignalを持つためのイベントを作成する
    HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent != nullptr);

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
    D3D12_ROOT_PARAMETER rootParameters[4] = {};
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

#pragma  region//BlendStateの設定を行う

    //書き込む色要素を決めることなども出来る
    D3D12_BLEND_DESC blendDesc{};
    //全ての色要素を書き込む
    blendDesc.RenderTarget[0].RenderTargetWriteMask =
        D3D12_COLOR_WRITE_ENABLE_ALL;

    Log(logStream, "SetBlendState");

#pragma endregion

#pragma  region//RasterizerStateの設定を行う
    //三角形の内部をピクセルに分解して、PixelShaderを起動する

    //RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    //裏面（時計回り）を表示しない　裏面をカウリング
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    //三角形の中を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    Log(logStream, "SetRasterizerState");


#pragma endregion

#pragma region//ShaderをCompileする

    //Shaderをコンパイルする
    Microsoft::WRL::ComPtr <IDxcBlob> vertexShaderBlob = CompileShader(L"Object3D.VS.hlsl",
        L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(vertexShaderBlob != nullptr);

    Log(logStream, "CompileVertexShader");

    Microsoft::WRL::ComPtr <IDxcBlob>pixelShaderBlob = CompileShader(L"Object3D.PS.hlsl",
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

    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();//RootSignature
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;//InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
    vertexShaderBlob->GetBufferSize() };//VertexShader
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
    pixelShaderBlob->GetBufferSize() };//PixelShader
    graphicsPipelineStateDesc.BlendState = blendDesc;//BlendState
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;//RasterizerState
    //書き込むRTVの情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    //利用するトポロジ（形状）のタイプ。三角形
    graphicsPipelineStateDesc.PrimitiveTopologyType =
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    //どのように画面に色を打ち込むかの設定（気にしなくていい）
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    //DepthStencilの設定
    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    //実際に生成
    Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
    hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
        IID_PPV_ARGS(&graphicsPipelineState));
    assert(SUCCEEDED(hr));

    Log(logStream, "CreatePSO");

#pragma endregion

#pragma region //Sprite 用の頂点リソースを作る
    //VertexResourceとVertexBufferViewを用意 矩形を表現するための三角形を二つ(頂点6つ)
    Microsoft::WRL::ComPtr <ID3D12Resource> vertexResourceSprite = CreateBufferResource(device, sizeof(VertexData) * 6);

    Log(logStream, "Create Sprite VertexResource");

    //頂点バッファビューを作成する
    D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
    //リソースの先頭アドレスから使う
    vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
    //使用するリソースのサイズ頂点6つ分のサイズ
    vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
    //1頂点あたりのサイズ
    vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

    Log(logStream, "Create Sprite VertexBufferView");

#pragma endregion

#pragma region //Texrureを読んで転送する
    DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = CreateTextureResource(device, metadata);
    /* UploadTextureData(textureResource, mipImages);*/
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = UploadTextureData(textureResource.Get(), mipImages, device, commandList);

    //2枚目のテクスチャを読む
    DirectX::ScratchImage mipImages2 = LoadTexture("resources/monsterBall.png");
    const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
    Microsoft::WRL::ComPtr<ID3D12Resource>textureResource2 = CreateTextureResource(device, metadata2);
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource2 = UploadTextureData(textureResource2.Get(), mipImages2, device, commandList);

#pragma endregion

#pragma region ShaderResourceViewを作る
    //metaDataを基にSRVの設定
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//texture
    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

    //SRVを作成するDescriptorHeapの場所の選択
    D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = GetCPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, 1);
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = GetGPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, 1);

    //SRVの生成
    device->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

    //二枚目
    //metaDataを基にSRVの設定
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
    srvDesc2.Format = metadata2.format;
    srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//texture
    srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

    //SRVを作成するDescriptorHeapの場所の選択
    D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, 2);
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, 2);

    //SRVの生成
    device->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);

#pragma endregion

#pragma region//Resourceにデータを書き込む

    //モデルの読み込み
    ModelData modelData = LoadObjeFile("resources", "plane.obj");
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

#pragma endregion

#pragma region //Sprite用の頂点データの設定
    VertexData* vertexDataSprite = nullptr;
    vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
    //1枚目の三角形 四頂点でスプライト描画が完成
    vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };//左下
    vertexDataSprite[0].texcoord = { 0.0f,1.0f };
    vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };//法線
    vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };//左上
    vertexDataSprite[1].texcoord = { 0.0f,0.0f };
    vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };
    vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };//右下
    vertexDataSprite[2].texcoord = { 1.0f,1.0f };
    vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };
    vertexDataSprite[3].position = { 640.0f,0.0f,0.0f,1.0f };//右上
    vertexDataSprite[3].texcoord = { 1.0f,0.0f };
    vertexDataSprite[3].normal = { 0.0f,0.0f,-1.0f };
#pragma endregion

#pragma region//IndexResourceSpriteを作成
    Microsoft::WRL::ComPtr <ID3D12Resource> indexResourceSprite = CreateBufferResource(device, sizeof(uint32_t) * 6);
    //Viewを作成する IndexBufferView(IBV)
    D3D12_INDEX_BUFFER_VIEW  indexBufferViewSprite{};
    //リソースの先頭アドレスから使う
    indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
    //使用するリソースのサイズはインデックス6つ分のサイズ
    indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
    //インデックスはuint32_tとする
    indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;
#pragma endregion

#pragma region//IndexResourceにデータを書き込む
    //インデックスリーソースにデータを書き込む
    uint32_t* indexDataSprite = nullptr;
    indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));

    //頂点数を削減
    indexDataSprite[0] = 0;
    indexDataSprite[1] = 1;
    indexDataSprite[2] = 2;

    indexDataSprite[3] = 1;
    indexDataSprite[4] = 3;
    indexDataSprite[5] = 2;

#pragma endregion

#pragma region//Material用のResourceを作る
    //マテリアル用のリソースを作る。
    Microsoft::WRL::ComPtr <ID3D12Resource> materialResource = CreateBufferResource(device, sizeof(Material));
    //マテリアルにデータを書き込む
    Material* materialData = nullptr;
    //書き込むためのアドレスを取得
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    //今回は白
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData->enableLighting = true;
    materialData->uvTransform = MakeIdentity4x4();
    Log(logStream, "MakeResourceForMaterial");

    //Sprite用のマテリアルを作成
    //マテリアル用のリソースを作る。
    Microsoft::WRL::ComPtr <ID3D12Resource> materialResourceSprite = CreateBufferResource(device, sizeof(Material));
    //マテリアルにデータを書き込む
    Material* materialDataSprite = nullptr;
    //書き込むためのアドレスを取得
    materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
    materialDataSprite->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialDataSprite->enableLighting = false;
    materialDataSprite->uvTransform = MakeIdentity4x4();

    Log(logStream, "MakeResourceForMaterialSprite");

#pragma endregion

#pragma region//UVTransform
    Transform uvTransformSprite{
        {1.0f,1.0f,1.0f},
        {0.0f,0.0f,0.0f},
        {0.0f,0.0f,0.0f},
    };

    Matrix4x4 uvTransformMatrix = MakeIdentity4x4();

#pragma endregion

#pragma region//TransformationMatrix用のResourceを作る

    //WVP用のリソースを作る。Matrix3x3 1つ分のサイズを用意する。
    Microsoft::WRL::ComPtr <ID3D12Resource> wvpResource = CreateBufferResource(device, sizeof(TransformationMatrix));
    //データを書き込む
    TransformationMatrix* wvpDate = nullptr;
    //書き込むためのアドレスを取得
    wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpDate));

    //三角形の座標
    Transform transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
    //三角形の行列
    Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
    //カメラ座標
    Transform cameraTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-5.0f} };
    //カメラ行列
    Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
    //カメラの逆行列
    Matrix4x4 viewMatrix = Inverse(cameraMatrix);
    //透視投影行列
    Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kClientWidth) / float(kClientHeight), 0.1f, 100.0f);
    //WVpMatrixを作る
    Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

    *wvpDate = { worldViewProjectionMatrix,worldMatrix };

    Log(logStream, "MakeResourceForTransformationMatrix");

#pragma endregion

#pragma region//Sprite用のTransformationMatrix用のリソースを作る。
    //Matrix4x4　1つ分のサイズを用意
    Microsoft::WRL::ComPtr <ID3D12Resource> transformationMatrixResourceSprite = CreateBufferResource(device, sizeof(TransformationMatrix));
    //データを書き込む
    TransformationMatrix* transformationMatrixDataSprite = nullptr;
    //書き込むためのアドレスを取得
    transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));

    Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
    Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
    Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
    //平行投影のためOrthographicを利用している
    Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(kClientWidth), float(kClientHeight), 0.0f, 100.0f);
    Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
    *transformationMatrixDataSprite = { worldViewProjectionMatrixSprite, worldMatrixSprite };

#pragma endregion

#pragma region//stencileTextureResourceの作成
    Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilResource = CreateDepthStencileTextureResource(device, kClientWidth, kClientHeight);

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
    //ビューポート
    D3D12_VIEWPORT viewport{};
    //クライアント領域のサイズと一緒にして画面全体に表示
    viewport.Width = kClientWidth;
    viewport.Height = kClientHeight;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    //シザー矩形
    D3D12_RECT scissorRect{};
    //基本的にビューポートと同じ矩形が構成されるようにする
    scissorRect.left = 0;
    scissorRect.right = kClientWidth;
    scissorRect.top = 0;
    scissorRect.bottom = kClientHeight;

    Log(logStream, "ViewportAndScissor");

#pragma endregion

#pragma region//ImGuiの初期化。
#ifdef _DEBUG
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(device.Get(),
        swapChainDesc.BufferCount,
        rtvDesc.Format,
        srvDescriptorHeap.Get(),
        srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
    Log(logStream, "InitImGui");
#endif
#pragma endregion

    bool useMonsterBall = true;

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

#pragma region//ImGuiにここからフレームが始まる旨を伝える

#ifdef _DEBUG
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            debugCamera.Update();
#endif

#pragma endregion

            //ゲームの処理


#pragma region //ゲームの処理

#ifdef _DEBUG

            //Lightを設定
            ImGui::Begin("DirectionalLight");
            ImGui::DragFloat4("color", &directionalLightData->color.x);
            Vector3 direction = directionalLightData->direction;
            ImGui::SliderFloat3("direction2", &direction.x, -1.0f, 1.0f);//後で正規化する
            directionalLightData->direction = Normalize(direction);

            ImGui::DragFloat("intensity", &directionalLightData->intensity);
            ImGui::End();

#pragma region//Modelのデバッグ 

            //開発用のUIの処理。実際に開発用のUIを出す場合はここkをゲーム固有の処理に置き換える
         /*   ImGui::ShowDemoWindow();*/
            ImGui::Begin("Model");
            ImGui::Checkbox("useMonsterBall", &useMonsterBall);
            ImGui::SliderFloat3("scale", &transform.scale.x, 0.0f, 8.0f);
            ImGui::SliderFloat3("rotate", &transform.rotate.x, 0.0f, 360.0f);
            ImGui::SliderFloat3("translate", &transform.translate.x, -2.0f, 2.0f);
            ImGui::ColorPicker4("materialColor", &(materialData->color.x));
            ImGui::DragFloat4("vertexData0", &(vertexData[0].position.x));
            ImGui::DragFloat4("vertexData1", &(vertexData[1].position.x));
            ImGui::DragFloat4("vertexData2", &(vertexData[2].position.x));

            if (ImGui::Button("Init")) {
                transform.scale = { 1.0f, 1.0f, 1.0f };
                transform.rotate = { 0.0f };
                transform.translate = { 0.0f };
            }

            ImGui::End();

#pragma endregion

#pragma region//Spriteのデバッグ

            ImGui::Begin("Sprite");

            ImGui::SliderFloat3("scale", &transformSprite.scale.x, 0.0f, 4.0f);
            ImGui::SliderFloat3("rotate", &transformSprite.rotate.x, 0.0f, 360.0f);
            ImGui::SliderFloat3("translate", &transformSprite.translate.x, -128.0f, 1280.0f);
            ImGui::ColorPicker4("materialColor", &(materialDataSprite->color.x));
            ImGui::DragFloat2("uv : translate", &(uvTransformSprite.translate.x), 0.01f, -10.0f, 10.0f);
            ImGui::DragFloat2("uv : scale", &(uvTransformSprite.scale.x), 0.01f, -10.0f, 10.0f);
            ImGui::SliderAngle("uv : rotate", &(uvTransformSprite.rotate.z));

            ImGui::End();

#pragma endregion

#endif

            if (input.IsTriggerKey(DIK_1)) {
                //音声再生
                sound.SoundPlayWave(soundData1);
                useMonsterBall = (useMonsterBall) ? false : true;
            }

            if (input.IsTriggerKey(DIK_2)) {
                //音声再生
                sound.SoundPlayMP3(soundData2);
            }

#pragma region//UVの更新処理
            uvTransformMatrix = MakeAffineMatrix(uvTransformSprite.scale, uvTransformSprite.rotate, uvTransformSprite.translate);
            materialDataSprite->uvTransform = uvTransformMatrix;
#pragma endregion

#pragma region //Modelの更新

            //Model行列
            worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
            //カメラ座標
            cameraTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };
            //カメラ行列
            cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
            //カメラの逆行列
            viewMatrix = Inverse(cameraMatrix);
            //透視投影行列
            projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kClientWidth) / float(kClientHeight), 0.1f, 100.0f);
            ////WVpMatrixを作る
            //worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
            //WVpMatrixを作る DebugCameraに切り替え
            worldViewProjectionMatrix = Multiply(worldMatrix, debugCamera.GetViewProjectionMatrix());

            //データを書き込む
            *wvpDate = { worldViewProjectionMatrix,worldMatrix };

#pragma endregion

#pragma region//Spriteの更新処理

            worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
            viewMatrixSprite = MakeIdentity4x4();
            //平行投影のためOrthographicを利用している
            projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(kClientWidth), float(kClientHeight), 0.0f, 100.0f);
            worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
            *transformationMatrixDataSprite = { worldViewProjectionMatrixSprite,worldMatrixSprite };

#pragma endregion

#ifdef _DEBUG
            //ImGuiの内部コマンドを生成する
            ImGui::Render();
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
            UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

            //TransitionBarrierの設定
            D3D12_RESOURCE_BARRIER barrier{};
            //今回はバリアはTransition
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            //Noneにしておく
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            //バリアを張る対象のリソース。現在のバックバッファに対して行う
            barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
            //遷移前（現在）のResourceState
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            //遷移後のResourceState
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            //TransitionBarrierを張る
            commandList->ResourceBarrier(1, &barrier);

            //2.描画用のRTVとDSVを設定する
            D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
            //3.指定した色で画面全体をクリアする
            float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };//青っぽい色。RGBAの順
            commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
            //指定した深度で画面全体をクリアする
            commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


            //描画用のDescriptorHeapの設定
            ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap.Get() };
            commandList->SetDescriptorHeaps(1, descriptorHeaps);

#pragma region //Modelを描画する

            //ファイルへのログ出力
            Log(logStream, "DrawModel");

            commandList->RSSetViewports(1, &viewport);//Viewportを設定
            commandList->RSSetScissorRects(1, &scissorRect);//Scirssorを設定
            //RootSignatureを設定。PSOに設定しているけど別途設定が必要
            commandList->SetGraphicsRootSignature(rootSignature.Get());
            commandList->SetPipelineState(graphicsPipelineState.Get());//PSOを設定
            commandList->IASetVertexBuffers(0, 1, &vertexBufferView);//VBVを設定
            //形状を設定。PSOに設定している物とはまた別。同じものを設定すると考えておけばよい。
            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            //マテリアルCBufferの場所を設定　/*RotParameter配列の0番目 0->register(b4)1->register(b0)2->register(b4)*/
            commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
            Log(logStream, "SetMaterialToCBuffer");
            //wvp用のCBufferの場所を設定
            commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
            Log(logStream, "SetWVPToCBuffer");
            //SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
            commandList->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);

            //LightのCBufferの場所を設定
            commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

            //描画!(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
            commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

#pragma endregion

#pragma region//IndexSpriteの描画

            //頂点バッファビューを設定
            commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);//VBVを設定
            //IBVを設定new
            commandList->IASetIndexBuffer(&indexBufferViewSprite);//IBVを設定
            //マテリアルCBufferの場所を設定　/*RotParameter配列の0番目 0->register(b4)1->register(b0)2->register(b4)*/
            commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
            //TransformationMatrixCBufferの場所を設定
            commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
            //SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
            commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
            //描画!（DrawCall/ドローコール）6個のインデックスを使用し1つのインスタンスを描画。その他は当面0で良い。
            commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
            ////描画！（DrawCall/ドローコール）
            //commandList->DrawInstanced(6, 1, 0, 0);
#pragma endregion

#ifdef _DEBUG
            //諸々の描画処理が終了下タイミングでImGuiの描画コマンドを積む
            //実際のcommandListのImGuiの描画コマンドを積む
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());

#endif // _DEBUG


            //画面に書く処理は終わり、画面に移すので、状態を遷移
            //今回はRenderTargetからPresentにする
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            //　TransitionBarrierを張る
            commandList->ResourceBarrier(1, &barrier);


            //4.コマンドリストの内容を確定させる。全てのコマンドを詰んでから　Closesすること。
            hr = commandList->Close();
            assert(SUCCEEDED(hr));

            Log(logStream, "CloseCommandList");

            //5.GPUにコマンドリストの実行を行わせる
            ID3D12CommandList* commandLists[] = { commandList.Get() };
            commandQueue->ExecuteCommandLists(1, commandLists);
            //6.GPUとOSに画面の交換を行うよう通知する
            swapChain->Present(1, 0);

            //画面の更新が終わった直後GPUにシグナルを送る
            //Fenceの値を更新
            fenceValue++;
            //GPUがここまでたどり着いた時、Fenceの値を指定した値に代入するようにSignalを送る
            commandQueue->Signal(fence.Get(), fenceValue);

            //Fenceの値が指定したSignal値にたどり着いているか確認する GPUの処理を待つ
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
            hr = commandList->Reset(commandAllocator.Get(), nullptr);
            assert(SUCCEEDED(hr));

#pragma endregion

        }



    }

#ifdef _DEBUG

    //ImGuiの終了処理 ゲームループが終わったら行う
    //初期化と逆順に行う
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

#endif

#pragma region //解放処理

    CloseHandle(fenceEvent);

    //音声データの解放
    sound.SoundUnload(&soundData1);
    sound.SoundUnloadMP3(&soundData2);
    //xAudio2のReset
    sound.Reset();

    CloseWindow(hwnd);

    CoUninitialize();
#pragma endregion



    return 0;
}
