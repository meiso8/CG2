#include "../Header/CommandQueue.h"

#include<cassert> //assertも利用するため
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

void CommandQueue::Create(const Microsoft::WRL::ComPtr<ID3D12Device>& device) {

    //コマンドキューを生成する まとまった命令群を送るため
    HRESULT result = device->CreateCommandQueue(&commandQueueDesc_, IID_PPV_ARGS(&commandQueue_));

    //コマンドキューの生成が上手くいかなかったので起動できない
    assert(SUCCEEDED(result));

}
