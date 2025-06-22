#include "../Header/Sprite.h"
#include"../Header/CreateBufferResource.h"
#include"../Header/TransformationMatrix.h"
#include"../Header/math/MakeAffineMatrix.h"
#include"../Header/math/MakeIdentity4x4.h"
#include"../Header/math/Multiply.h"

void Sprite::Create(const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera& camera) {

    camera_ = &camera;

    CreateVertex(device);
    CreateIndexResource(device);
    CreateTransformationMatrix(device);
    CreateMaterial(device);

    uvTransform_ = {
          {1.0f,1.0f,1.0f},
          {0.0f,0.0f,0.0f},
          {0.0f,0.0f,0.0f},
    };

    uvTransformMatrix_ = MakeIdentity4x4();

}

void Sprite::CreateVertex(const Microsoft::WRL::ComPtr<ID3D12Device>& device) {

    //VertexResourceとVertexBufferViewを用意 矩形を表現するための三角形を二つ(頂点4つ)
    vertexResource_ = CreateBufferResource(device, sizeof(VertexData) * 4);

    //頂点バッファビューを作成する
    //リソースの先頭アドレスから使う
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    //使用するリソースのサイズ頂点4つ分のサイズ
    vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
    //1頂点あたりのサイズ
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

#pragma region //Sprite用の頂点データの設定

    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
    //1枚目の三角形 四頂点でスプライト描画が完成
    vertexData_[0].position = { 0.0f,360.0f,0.0f,1.0f };//左下
    vertexData_[0].texcoord = { 0.0f,1.0f };
    vertexData_[0].normal = { 0.0f,0.0f,-1.0f };//法線
    vertexData_[1].position = { 0.0f,0.0f,0.0f,1.0f };//左上
    vertexData_[1].texcoord = { 0.0f,0.0f };
    vertexData_[1].normal = { 0.0f,0.0f,-1.0f };
    vertexData_[2].position = { 640.0f,360.0f,0.0f,1.0f };//右下
    vertexData_[2].texcoord = { 1.0f,1.0f };
    vertexData_[2].normal = { 0.0f,0.0f,-1.0f };
    vertexData_[3].position = { 640.0f,0.0f,0.0f,1.0f };//右上
    vertexData_[3].texcoord = { 1.0f,0.0f };
    vertexData_[3].normal = { 0.0f,0.0f,-1.0f };

#pragma endregion

}

void Sprite::CreateIndexResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device) {

#pragma region//IndexResourceを作成
    indexResource_ = CreateBufferResource(device, sizeof(uint32_t) * 6);
    //Viewを作成する IndexBufferView(IBV)

    //リソースの先頭アドレスから使う
    indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
    //使用するリソースのサイズはインデックス6つ分のサイズ
    indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
    //インデックスはuint32_tとする
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
#pragma endregion

#pragma region//IndexResourceにデータを書き込む
    //インデックスリーソースにデータを書き込む
    indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));

    //頂点数を削減
    indexData_[0] = 0;
    indexData_[1] = 1;
    indexData_[2] = 2;

    indexData_[3] = 1;
    indexData_[4] = 3;
    indexData_[5] = 2;

#pragma endregion
}

void Sprite::CreateTransformationMatrix(const Microsoft::WRL::ComPtr<ID3D12Device>& device) {

    //Matrix4x4　1つ分のサイズを用意
    transformationMatrixResource_ = CreateBufferResource(device, sizeof(TransformationMatrix));
    //データを書き込む
    //書き込むためのアドレスを取得
    transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));

    transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
    worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    worldViewProjectionMatrix_ = Multiply(worldMatrix_, camera_->GetViewProjectionMatrix());
    *transformationMatrixData_ = { worldViewProjectionMatrix_, worldMatrix_ };
}

void Sprite::CreateMaterial(const Microsoft::WRL::ComPtr<ID3D12Device>& device) {

    //マテリアルリソースを作成
    materialResource_.CreateMaterial(device, false);

}

void Sprite::Update() {

    UpdateUV();

    worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    worldViewProjectionMatrix_ = Multiply(worldMatrix_, camera_->GetViewProjectionMatrix());
    *transformationMatrixData_ = { worldViewProjectionMatrix_,worldMatrix_ };
}

void Sprite::UpdateUV() {

    uvTransformMatrix_ = MakeAffineMatrix(uvTransform_.scale, uvTransform_.rotate, uvTransform_.translate);
    materialResource_.SetUV(uvTransformMatrix_);
}

void Sprite::Draw(
    CommandList& commandList,

    ShaderResourceView& srv
) {

    //頂点バッファビューを設定
    commandList.GetComandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);//VBVを設定
    //IBVを設定new
    commandList.GetComandList()->IASetIndexBuffer(&indexBufferView_);//IBVを設定
    //マテリアルCBufferの場所を設定　/*RotParameter配列の0番目 0->register(b4)1->register(b0)2->register(b4)*/
    commandList.GetComandList()->SetGraphicsRootConstantBufferView(0, materialResource_.GetMaterialResource()->GetGPUVirtualAddress());
    //TransformationMatrixCBufferの場所を設定
    commandList.GetComandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
    //SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
    commandList.GetComandList()->SetGraphicsRootDescriptorTable(2, srv.GetTextureSrvHandleGPU());

    //描画!（DrawCall/ドローコール）6個のインデックスを使用し1つのインスタンスを描画。その他は当面0で良い。
    commandList.GetComandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
};

