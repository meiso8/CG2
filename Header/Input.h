#pragma once
#define DIRECTINPUT_VERSION 0x0800//DirectXバージョン指定
//　DIRECTINPUT_VERSION　dinput.hのインクルードより上に書くこと。
#include <dinput.h>
#include<cstdint>

class Input {
public:
    HRESULT Initalize(const WNDCLASS& wc, const HWND& hwnd);

    /// @brief キーを押した状態 
    bool IsPressStateKey(const uint8_t& key);
    /// @briefキーを押した瞬間
    bool IsTriggerKey(const uint8_t& key);
    /// @briefキーを離した状態
    bool IsReleaseStateKey(const uint8_t& key);
    /// @briefキーを離した瞬間
    bool IsReleaseKey(const uint8_t& key);

    /// @brief キーの情報を取得する
    void InputInfoGet();

public:
    IDirectInputDevice8* keyboard_ = nullptr;
    //全キー入力状態を取得する
    BYTE key_[256] = {};
    BYTE preKey_[256] = {};
};