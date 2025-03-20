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

    MSG msg{};
    //ウィンドウのxボタンが押されるまでループ
    while (msg.message != WM_QUIT) {
        //Windowにメッセージが来ていたら最優先で処理させる
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            //ゲームの処理

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



    return 0;
}


