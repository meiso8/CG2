#include"../Header/Input.h"
#include <assert.h>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

#include"../Header/Log.h"
#define FPS 120

HRESULT Input::Initialize(Window& window) {

    HRESULT result;
    //DirectInputの初期化 ゲームパッドを追加するにしてもこのオブジェクトは一つでよい。
    IDirectInput8* directInput = nullptr;
    result = DirectInput8Create(
        window.GetWindowClass().hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
        (void**)&directInput, nullptr
    );
    assert(SUCCEEDED(result));

    //キーボードデバイスの生成
    result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
    assert(SUCCEEDED(result));

    //入力データ形式のセット
    result = keyboard_->SetDataFormat(&c_dfDIKeyboard);//標準形式 キーボードの場合
    assert(SUCCEEDED(result));

    //排他制御レベルのセット
    result = keyboard_->SetCooperativeLevel(
        window.GetHwnd(),
        DISCL_FOREGROUND//画面が手前にある場合のみ入力を受け付ける
        | DISCL_NONEXCLUSIVE //デバイスをこのアプリだけで占有しない
        | DISCL_NOWINKEY//Windowsキーを無効にする
    );
    assert(SUCCEEDED(result));

    //マウスデバイスの生成
    result = directInput->CreateDevice(GUID_SysMouse, &mouse_, NULL);
    assert(SUCCEEDED(result));

    // マウス用のデータ形式のセット
    result = mouse_->SetDataFormat(&c_dfDIMouse);
    assert(SUCCEEDED(result));

    // モードを設定（フォアグラウンド＆非排他モード）
    result = mouse_->SetCooperativeLevel(
        window.GetHwnd(),
        DISCL_FOREGROUND//画面が手前にある場合のみ入力を受け付ける
        | DISCL_NONEXCLUSIVE //デバイスをこのアプリだけで占有しない
        | DISCL_NOWINKEY//Windowsキーを無効にする
    );
    assert(SUCCEEDED(result));

    // デバイスの設定
    DIPROPDWORD diprop;
    diprop.diph.dwSize = sizeof(diprop);
    diprop.diph.dwHeaderSize = sizeof(diprop.diph);
    diprop.diph.dwObj = 0;
    diprop.diph.dwHow = DIPH_DEVICE;
    diprop.dwData = DIPROPAXISMODE_REL;	// 相対値モードで設定（絶対値はDIPROPAXISMODE_ABS）

    result = mouse_->SetProperty(DIPROP_AXISMODE, &diprop.diph);
    assert(SUCCEEDED(result));

    return result;

};

bool Input::IsPushKey(const uint8_t& keyNum) {

    if (key_[keyNum]) {
        return true;
    }

    return false;

}
bool Input::IsTriggerKey(const uint8_t& keyNum) {

    if (key_[keyNum] && !preKey_[keyNum]) {
        return true;
    }

    return false;

}
bool Input::IsReleaseStateKey(const uint8_t& keyNum) {

    if (!key_[keyNum]) {
        return true;
    }

    return false;

}
bool Input::IsReleaseKey(const uint8_t& keyNum) {

    if (!key_[keyNum] && preKey_[keyNum]) {
        return true;
    }

    return false;

}

void Input::InputInfoGet() {

    //キーの状態をコピーする
    memcpy(preKey_, key_, 256);
    //キーボード情報の取得開始
    keyboard_->Acquire();
    //全キー入力状態を取得する
    keyboard_->GetDeviceState(sizeof(key_), key_);

    //マウスの状態をコピーする
    memcpy(&mouseState_bak_, &mouseState_, sizeof(mouseState_bak_));
    // 入力制御開始
    mouse_->Acquire();
    //マウスの状態を取得する
    mouse_->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState_);
}

bool Input::IsPressMouse(uint32_t index) {
    return (mouseState_.rgbButtons[index] & 0x80) ? true : false;
}

Vector2& Input::GetMousePos() {
    static Vector2 mousePos; // 静的変数を使用して左辺値を確保  
    mousePos.x = static_cast<float>(mouseState_.lX);
    mousePos.y = static_cast<float>(mouseState_.lY);
    return mousePos;
}

float Input::GetMouseWheel() {
    mouseWheelVol_ += static_cast<float>(mouseState_.lZ) / FPS;
    return mouseWheelVol_;
};

Input::~Input() {

    mouse_->Unacquire();
    mouse_->Release();
    mouse_ = nullptr;

}