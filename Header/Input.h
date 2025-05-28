#pragma once
#define DIRECTINPUT_VERSION 0x0800//DirectXバージョン指定
//　DIRECTINPUT_VERSION　dinput.hのインクルードより上に書くこと。
#include <dinput.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#include <assert.h>

class Input {
public:
    HRESULT Initalize(const WNDCLASS& wc, const HWND& hwnd) {

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

    /// @brief キーを押した状態
    /// @param keyNum 
    /// @return 
    bool IsPressStateKey(const uint8_t& keyNum) {

        if (key_[keyNum]) {
            return true;
        }

        return false;

    }

    //キーを押した瞬間
    bool IsTriggerKey(const uint8_t& keyNum) {

        if (key_[keyNum] && !preKey_[keyNum]) {
            return true;
        }

        return false;

    }

    //キーを離した状態
    bool IsReleaseStateKey(const uint8_t& keyNum) {

        if (!key_[keyNum]) {
            return true;
        }

        return false;

    }

    //キーを離した瞬間
    bool IsReleaseKey(const uint8_t& keyNum) {

        if (!key_[keyNum]&& preKey_[keyNum]) {
            return true;
        }

        return false;

    }


    void InputInfoGet() {

        //キーの状態をコピーする
        memcpy(preKey_, key_, 256);
        //キーボード情報の取得開始
        keyboard_->Acquire();
        //全キー入力状態を取得する
        keyboard_->GetDeviceState(sizeof(key_), key_);

    }

public:
    IDirectInputDevice8* keyboard_ = nullptr;
    //全キー入力状態を取得する
    BYTE key_[256] = {};
    BYTE preKey_[256] = {};
};