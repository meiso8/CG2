#include "../Header/Model.h"
#include"../Header/CreateBufferResource.h"
#include"../Header/Texture.h"
#include"../Header/TransformationMatrix.h"
#include"../Header/math/MakeAffineMatrix.h"
#include"../Header/math/Multiply.h"
#include"../Header/math/Transform.h"
#include<numbers>

void Model::Create(const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera& camera) {

    camera_ = &camera;

    //マテリアルの作成
    materialResource_.CreateMaterial(device, true);

    CreateWorldVP(device);

    //CreateVertex(device);

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

//
//void Model::CreateVertex(const Microsoft::WRL::ComPtr<ID3D12Device>& device) {
//
//    //頂点リソースを作る
//    vertexResource_ = CreateBufferResource(device, sizeof(VertexData) * modelData_.vertices.size());
//
//    //頂点バッファビューを作成する
//    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();//リソースの先頭アドレスから使う
//    vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());//使用するリソースのサイズは頂点のサイズ
//    vertexBufferView_.StrideInBytes = sizeof(VertexData);//1頂点あたりのサイズ
//
//    //頂点リソースにデータを書き込む
//    VertexData* vertexData = nullptr;
//    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));//書き込むためのアドレスを取得
//    std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());//頂点データをリソースにコピー
//
//};


void Model::Update() {

    //Model行列
    worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    //WVpMatrixを作る 
    worldViewProjectionMatrix_ = Multiply(worldMatrix_, camera_->GetViewProjectionMatrix());
    //データを書き込む
    *wvpDate_ = { worldViewProjectionMatrix_,worldMatrix_ };
}

void Model::Draw(
    CommandList& commandList,
    D3D12_VERTEX_BUFFER_VIEW& vertexBufferView,
    ShaderResourceView& srv
) {
    commandList.GetComandList()->IASetVertexBuffers(0, 1, &vertexBufferView);//VBVを設定
    //マテリアルCBufferの場所を設定　/*RotParameter配列の0番目 0->register(b4)1->register(b0)2->register(b4)*/
    commandList.GetComandList()->SetGraphicsRootConstantBufferView(0, materialResource_.GetMaterialResource()->GetGPUVirtualAddress());
    //wvp用のCBufferの場所を設定
    commandList.GetComandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
    //SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
    commandList.GetComandList()->SetGraphicsRootDescriptorTable(2, srv.GetTextureSrvHandleGPU());

}

void Model::DrawCall(CommandList& commandList, ModelData& modelData) {

    //描画!(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
    commandList.GetComandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

}