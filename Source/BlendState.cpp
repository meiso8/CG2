

#include "../Header/BlendState.h"

void BlendState::Create() {

    //全ての色要素を書き込む
    blendDesc_.RenderTarget[0].RenderTargetWriteMask =
        D3D12_COLOR_WRITE_ENABLE_ALL;

};
