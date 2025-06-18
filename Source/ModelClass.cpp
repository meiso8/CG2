#include "../Header/ModelClass.h"

void ModelClass::Draw(
    CommandList& commandList,
    D3D12_VERTEX_BUFFER_VIEW& vertexBufferView,
    const Microsoft::WRL::ComPtr <ID3D12Resource>& materialResource,
    const Microsoft::WRL::ComPtr <ID3D12Resource>& wvpResource,
    ShaderResourceView& srv
) {
    commandList.GetComandList()->IASetVertexBuffers(0, 1, &vertexBufferView);//VBVを設定
    //マテリアルCBufferの場所を設定　/*RotParameter配列の0番目 0->register(b4)1->register(b0)2->register(b4)*/
    commandList.GetComandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
    //wvp用のCBufferの場所を設定
    commandList.GetComandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
    //SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
    commandList.GetComandList()->SetGraphicsRootDescriptorTable(2, srv.GetTextureSrvHandleGPU());

}

void ModelClass::DrawCall(CommandList& commandList, ModelData& modelData) {

    //描画!(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
    commandList.GetComandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

}