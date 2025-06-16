#include "../Header/RasterizerState.h"


void RasterizerState::Create() {
    //三角形の内部をピクセルに分解して、PixelShaderを起動する

    //裏面（時計回り）を表示しない　裏面をカウリング
    rasterizerDesc_.CullMode = D3D12_CULL_MODE_BACK;
    //三角形の中を塗りつぶす
    rasterizerDesc_.FillMode = D3D12_FILL_MODE_SOLID;

}

