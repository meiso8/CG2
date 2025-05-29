#include"../Header/Input.h"
#include <assert.h>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

HRESULT Input::Initalize(const WNDCLASS& wc, const HWND& hwnd) {

    HRESULT result;
    //DirectInputの初期化
    IDirectInput8* directInput = nullptr;
    result = DirectInput8Create(
        wc.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
        (void**)&directInput, nullptr
    );
    assert(SUCCEEDED(result));

    //キーボードデバイスの生成
    //IDirectInputDevice8* keyboard = nullptr;
    result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
    assert(SUCCEEDED(result));

    //入力データ形式のセット
    result = keyboard_->SetDataFormat(&c_dfDIKeyboard);//標準形式 キーボードの場合
    assert(SUCCEEDED(result));

    //排他制御レベルのセット
    result = keyboard_->SetCooperativeLevel(
        hwnd,
        DISCL_FOREGROUND//画面が手前にある場合のみ入力を受け付ける
        | DISCL_NONEXCLUSIVE //デバイスをこのアプリだけで占有しない
        | DISCL_NOWINKEY//Windowsキーを無効にする
    );
    assert(SUCCEEDED(result));

    return result;

};


bool Input::IsPressStateKey(const uint8_t& keyNum) {

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

}
