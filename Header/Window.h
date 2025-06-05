#pragma once  
#include <Windows.h>  
#include<cstdint>//int32_tを使うため

class Window
{
public:
    void Create(int32_t clientWidth, int32_t clientHeight);
    WNDCLASS GetWindowClass() { return wc_; }
    HWND GetHwnd() { return hwnd_; }
    int32_t GetClientWidth() const {
        return clientWidth_;
    }
    int32_t GetClientHeight() const {
        return clientHeight_;
    }

private:
    /// @brief ウィンドウクラス  
    WNDCLASS wc_{};
    /// @brief ウィンドウハンドル  
    HWND hwnd_{};
    //クライアント領域のサイズ  
    int32_t clientWidth_;
    int32_t clientHeight_;
};
