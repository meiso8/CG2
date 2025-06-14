#pragma once

#include <dxgidebug.h>//リソースリークチェックのため
//#pragma comment(lib,"dxguid.lib")

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