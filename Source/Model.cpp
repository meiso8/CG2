#include "../Header/Model.h"
#include"../Header/CreateBufferResource.h"
#include"../Header/Texture.h"
#include"../Header/TransformationMatrix.h"
#include"../Header/math/MakeAffineMatrix.h"
#include"../Header/math/Multiply.h"
#include"../Header/math/Transform.h"
#include<numbers>

Model::Model(
    Camera& camera,
    CommandList& commandList,
    D3D12_VIEWPORT& viewport,
    D3D12_RECT& scissorRect,
    const Microsoft::WRL::ComPtr<ID3D12RootSignature>& rootSignature,
    PSO& pso) {

    camera_ = &camera;
    commandList_ = &commandList;
    viewport_ = &viewport;
    scissorRect_ = &scissorRect;
    rootSignature_ = rootSignature;
    pso_ = &pso;
};

void Model::Create(
    const std::string& directoryPath,
    const std::string& filename,
    const Microsoft::WRL::ComPtr<ID3D12Device>& device,
    const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap,
    const uint32_t& descriptorSizeSRV
) {

    //マテリアルの作成
    materialResource_.CreateMaterial(device, true);

    CreateWorldVP(device);

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
    mipImages_ = LoadTexture(modelData_.material.textureFilePath);
    const DirectX::TexMetadata& metadata = mipImages_.GetMetadata();
    textureResource_ = CreateTextureResource(device, metadata);
    intermediateResource_ = UploadTextureData(textureResource_.Get(), mipImages_, device, commandList_->GetComandList());

    srv_.Create(metadata, textureResource_, 2, device, srvDescriptorHeap, descriptorSizeSRV);
}

void Model::CreateWorldVP(const Microsoft::WRL::ComPtr<ID3D12Device>& device) {
    //WVP用のリソースを作る。Matrix3x3 1つ分のサイズを用意する。
    wvpResource_ = CreateBufferResource(device, sizeof(TransformationMatrix));
    //データを書き込む
    //書き込むためのアドレスを取得
    wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpDate_));

    //三角形の座標
    transform_ = { {1.0f,1.0f,1.0f},{std::numbers::pi_v<float>*7.0f / 4.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
    //三角形の行列
    worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    //WVpMatrixを作る
    worldViewProjectionMatrix_ = Multiply(worldMatrix_, camera_->GetViewProjectionMatrix());
    *wvpDate_ = { worldViewProjectionMatrix_,worldMatrix_ };

};

void Model::InitTraslate() {

    transform_.scale = { 1.0f, 1.0f, 1.0f };
    transform_.rotate = { 0.0f };
    transform_.translate = { 0.0f };

};

void Model::Update() {

    //Model行列
    worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    //WVpMatrixを作る 
    worldViewProjectionMatrix_ = Multiply(worldMatrix_, camera_->GetViewProjectionMatrix());
    //データを書き込む
    *wvpDate_ = { worldViewProjectionMatrix_,worldMatrix_ };
}

void Model::PreDraw() {
    commandList_->GetComandList()->RSSetViewports(1, viewport_);//Viewportを設定
    commandList_->GetComandList()->RSSetScissorRects(1, scissorRect_);//Scirssorを設定
    //RootSignatureを設定。PSOに設定しているけど別途設定が必要
    commandList_->GetComandList()->SetGraphicsRootSignature(rootSignature_.Get());
    commandList_->GetComandList()->SetPipelineState(pso_->GetGraphicsPipelineState().Get());//PSOを設定
    //形状を設定。PSOに設定している物とはまた別。同じものを設定すると考えておけばよい。
    commandList_->GetComandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Model::Draw(

) {
    commandList_->GetComandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);//VBVを設定
    //マテリアルCBufferの場所を設定　/*RotParameter配列の0番目 0->register(b4)1->register(b0)2->register(b4)*/
    commandList_->GetComandList()->SetGraphicsRootConstantBufferView(0, materialResource_.GetMaterialResource()->GetGPUVirtualAddress());
    //wvp用のCBufferの場所を設定
    commandList_->GetComandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
    //SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
    commandList_->GetComandList()->SetGraphicsRootDescriptorTable(2, srv_.GetTextureSrvHandleGPU());
}

void Model::DrawCall() {

    //描画!(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
    commandList_->GetComandList()->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);

}