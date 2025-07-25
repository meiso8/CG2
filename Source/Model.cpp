#include "../Header/Model.h"
#include"../Header/CreateBufferResource.h"
#include"../Header/Texture.h"
#include"../Header/TransformationMatrix.h"
#include"../Header/math/MakeAffineMatrix.h"
#include"../Header/math/Multiply.h"
#include"../Header/math/Transform.h"
#include<numbers>

void Model::Create(
    const std::string& directoryPath,
    const std::string& filename,
    const Microsoft::WRL::ComPtr<ID3D12Device>& device,
    const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap) {

    //マテリアルの作成
    materialResource_.CreateMaterial(device, true);

    CreateWorldVPResource(device);

    //モデルの読み込み
    modelData_ = LoadObjeFile(directoryPath, filename);
    //頂点リソースを作る
    vertexResource_ = CreateBufferResource(device, sizeof(VertexData) * modelData_.vertices.size());

    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();//リソースの先頭アドレスから使う
    vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());//使用するリソースのサイズは頂点のサイズ
    vertexBufferView_.StrideInBytes = sizeof(VertexData);//1頂点あたりのサイズ

    //頂点リソースにデータを書き込む
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));//書き込むためのアドレスを取得
    std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());//頂点データをリソースにコピー

    //モデルのテクスチャを読む
    texture_ = new Texture(device, *modelConfig_.commandList);
    texture_->Load(modelData_.material.textureFilePath);

    //これだとダメだわ
    srv_.Create(*texture_, 3, device, srvDescriptorHeap);
}

void Model::CreateWorldVPResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device) {
    //WVP用のリソースを作る。Matrix3x3 1つ分のサイズを用意する。
    wvpResource_ = CreateBufferResource(device, sizeof(TransformationMatrix));
    //データを書き込む
    //書き込むためのアドレスを取得
    wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpDate_));
};

void Model::SetColor(const Vector4& color) {
    materialResource_.SetColor(color);
};

void Model::PreDraw() {
    modelConfig_.commandList->GetComandList()->RSSetViewports(1, modelConfig_.viewport);//Viewportを設定
    modelConfig_.commandList->GetComandList()->RSSetScissorRects(1, modelConfig_.scissorRect);//Scirssorを設定
    //RootSignatureを設定。PSOに設定しているけど別途設定が必要
    modelConfig_.commandList->GetComandList()->SetGraphicsRootSignature(modelConfig_.rootSignature->GetRootSignature().Get());
    modelConfig_.commandList->GetComandList()->SetPipelineState(modelConfig_.pso->GetGraphicsPipelineState().Get());//PSOを設定
    //形状を設定。PSOに設定している物とはまた別。同じものを設定すると考えておけばよい。
    modelConfig_.commandList->GetComandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Model::Draw(
    const Matrix4x4& worldMatrix, Camera& camera
) {

    worldViewProjectionMatrix_ = Multiply(worldMatrix, camera.GetViewProjectionMatrix());
    //データを書き込む
    *wvpDate_ = { worldViewProjectionMatrix_,worldMatrix };

    modelConfig_.commandList->GetComandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);//VBVを設定
    //マテリアルCBufferの場所を設定　/*RotParameter配列の0番目 0->register(b4)1->register(b0)2->register(b4)*/
    modelConfig_.commandList->GetComandList()->SetGraphicsRootConstantBufferView(0, materialResource_.GetMaterialResource()->GetGPUVirtualAddress());
    //wvp用のCBufferの場所を設定
    modelConfig_.commandList->GetComandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
    //SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
    modelConfig_.commandList->GetComandList()->SetGraphicsRootDescriptorTable(2, srv_.GetTextureSrvHandleGPU());
    //LightのCBufferの場所を設定
    modelConfig_.commandList->GetComandList()->SetGraphicsRootConstantBufferView(3, modelConfig_.directionalLightResource->GetGPUVirtualAddress());
    //timeのSRVの場所を設定
    modelConfig_.commandList->GetComandList()->SetGraphicsRootShaderResourceView(4, modelConfig_.waveResource->GetGPUVirtualAddress());
    //expansionのCBufferの場所を設定
    modelConfig_.commandList->GetComandList()->SetGraphicsRootConstantBufferView(5, modelConfig_.expansionResource->GetGPUVirtualAddress());
    //描画!(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
    modelConfig_.commandList->GetComandList()->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);

}

Model::~Model() {

    delete texture_;
};