#include "../Header/Cube.h"

#include"../Header/CreateBufferResource.h"


#include"../Header/math/MakeAffineMatrix.h"
#include"../Header/math/Multiply.h"

void Cube::Create(
    const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera& camera, ModelConfig& mc
) {

    camera_ = &camera;

    CreateVertex(device);
    CreateIndexResource(device);
    CreateTransformationMatrix(device);
    CreateMaterial(device);

    modelConfig_ = mc;

}

void Cube::CreateVertex(const Microsoft::WRL::ComPtr<ID3D12Device>& device) {

    vertexResource_ = CreateBufferResource(device, sizeof(VertexData) * 8);

    //頂点バッファビューを作成する
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(VertexData) * 8;
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

#pragma region //頂点データの設定

    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

    Vector3 min = { -0.5f,-0.5f,-0.5f };
    Vector3 max = { 0.5f,0.5f,0.5f };

    vertexData_[0].position = { (min,1.0f) };//左sita
    vertexData_[0].texcoord = { 0.0f,1.0f };
    vertexData_[0].normal = { vertexData_[0].position.x,  vertexData_[0].position.y,  vertexData_[0].position.z };//法線
    vertexData_[1].position = { min.x,max.y,min.z,1.0f };//左上
    vertexData_[1].texcoord = { 0.0f,0.0f };
    vertexData_[1].normal = { vertexData_[1].position.x,  vertexData_[1].position.y,  vertexData_[1].position.z };
    vertexData_[2].position = { max.x,min.y,min.z,1.0f };//右下
    vertexData_[2].texcoord = { 1.0f,1.0f };
    vertexData_[2].normal = { vertexData_[2].position.x,  vertexData_[2].position.y,  vertexData_[2].position.z };
    vertexData_[3].position = { max.x,max.y,min.z,1.0f };//右上
    vertexData_[3].texcoord = { 1.0f,0.0f };
    vertexData_[3].normal = { vertexData_[3].position.x,  vertexData_[3].position.y,  vertexData_[3].position.z };

    vertexData_[4].position = { min.x,min.y,max.z,1.0f };//左sita
    vertexData_[4].texcoord = { 0.0f,1.0f };
    vertexData_[4].normal = { vertexData_[4].position.x, vertexData_[4].position.y, vertexData_[4].position.z };//法線
    vertexData_[5].position = { min.x,max.y,max.z,1.0f };//hidariue
    vertexData_[5].texcoord = { 0.0f,0.0f };
    vertexData_[5].normal = { vertexData_[5].position.x, vertexData_[5].position.y, vertexData_[5].position.z };
    vertexData_[6].position = { max.x,min.y,max.z,1.0f };//migisita
    vertexData_[6].texcoord = { 1.0f,1.0f };
    vertexData_[6].normal = { vertexData_[6].position.x,  vertexData_[6].position.y,  vertexData_[6].position.z };
    vertexData_[7].position = { (max,1.0f) };//migiue
    vertexData_[7].texcoord = { 1.0f,0.0f };
    vertexData_[7].normal = { vertexData_[7].position.x,  vertexData_[7].position.y,  vertexData_[7].position.z };
#pragma endregion

}

//void Cube::SetVertexPos(const Vector3& start, const Vector3& end) {
//    vertexData_[0].position = { start.x,start.y,0.0f,1.0f };//左下
//    vertexData_[0].normal = { vertexData_[0].position.x,  vertexData_[0].position.y,  vertexData_[0].position.z };//法線
//    vertexData_[1].position = { start.x, end.y,0.0f,1.0f };//左上
//    vertexData_[1].normal = { vertexData_[1].position.x,  vertexData_[1].position.y,  vertexData_[1].position.z };
//    vertexData_[2].position = { end.x,start.y,0.0f,1.0f };//右下
//    vertexData_[2].normal = { vertexData_[2].position.x,  vertexData_[2].position.y,  vertexData_[2].position.z };
//    vertexData_[3].position = { end.x,end.y,0.0f,1.0f };//右上
//    vertexData_[3].normal = { vertexData_[3].position.x,  vertexData_[3].position.y,  vertexData_[3].position.z };
//};

void Cube::CreateIndexResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device) {

#pragma region//IndexResourceを作成
    indexResource_ = CreateBufferResource(device, sizeof(uint32_t) * 39);
    //Viewを作成する IndexBufferView(IBV)

    //リソースの先頭アドレスから使う
    indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes = sizeof(uint32_t) * 39;
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

    indexData_[6] = 2;
    indexData_[7] = 3;
    indexData_[8] = 5;

    indexData_[9] = 2;
    indexData_[10] = 3;
    indexData_[11] = 6;

    indexData_[12] = 3;
    indexData_[13] = 7;
    indexData_[14] = 6;

    indexData_[15] = 6;
    indexData_[16] = 7;
    indexData_[17] = 4;

    indexData_[18] = 7;
    indexData_[19] = 5;
    indexData_[20] = 4;

    indexData_[21] = 4;
    indexData_[22] = 0;
    indexData_[23] = 6;

    indexData_[24] = 2;
    indexData_[25] = 6;
    indexData_[26] = 0;

    indexData_[27] = 0;
    indexData_[28] = 1;
    indexData_[29] = 4;

    indexData_[30] = 4;
    indexData_[31] = 1;
    indexData_[32] = 5;

    indexData_[33] = 5;
    indexData_[34] = 7;
    indexData_[35] = 1;

    indexData_[36] = 1;
    indexData_[37] = 7;
    indexData_[38] = 3;

#pragma endregion
}

void Cube::CreateTransformationMatrix(const Microsoft::WRL::ComPtr<ID3D12Device>& device) {

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

void Cube::CreateMaterial(const Microsoft::WRL::ComPtr<ID3D12Device>& device) {

    //マテリアルリソースを作成
    materialResource_.CreateMaterial(device, false);

}

void Cube::SetColor(const Vector4& color) {
    materialResource_.SetColor(color);
}

void Cube::Update() {

    worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    worldViewProjectionMatrix_ = Multiply(worldMatrix_, camera_->GetViewProjectionMatrix());
    *transformationMatrixData_ = { worldViewProjectionMatrix_,worldMatrix_ };
}


void Cube::PreDraw() {
    modelConfig_.commandList->GetComandList()->RSSetViewports(1, modelConfig_.viewport);//Viewportを設定
    modelConfig_.commandList->GetComandList()->RSSetScissorRects(1, modelConfig_.scissorRect);//Scirssorを設定
    //RootSignatureを設定。PSOに設定しているけど別途設定が必要
    modelConfig_.commandList->GetComandList()->SetGraphicsRootSignature(modelConfig_.rootSignature->GetRootSignature().Get());
    modelConfig_.commandList->GetComandList()->SetPipelineState(modelConfig_.pso->GetGraphicsPipelineState().Get());//PSOを設定
    //形状を設定。PSOに設定している物とはまた別。同じものを設定すると考えておけばよい。
    modelConfig_.commandList->GetComandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Cube::Draw(
    ShaderResourceView& srv
) {
    //頂点バッファビューを設定
    modelConfig_.commandList->GetComandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);//VBVを設定
    //IBVを設定new
    modelConfig_.commandList->GetComandList()->IASetIndexBuffer(&indexBufferView_);//IBVを設定
    //マテリアルCBufferの場所を設定　/*RotParameter配列の0番目 0->register(b4)1->register(b0)2->register(b4)*/
    modelConfig_.commandList->GetComandList()->SetGraphicsRootConstantBufferView(0, materialResource_.GetMaterialResource()->GetGPUVirtualAddress());
    //TransformationMatrixCBufferの場所を設定
    modelConfig_.commandList->GetComandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
    //SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
    modelConfig_.commandList->GetComandList()->SetGraphicsRootDescriptorTable(2, srv.GetTextureSrvHandleGPU());
    //LightのCBufferの場所を設定
    modelConfig_.commandList->GetComandList()->SetGraphicsRootConstantBufferView(3, modelConfig_.directionalLightResource->GetGPUVirtualAddress());
    //timeのSRVの場所を設定
    modelConfig_.commandList->GetComandList()->SetGraphicsRootShaderResourceView(4, modelConfig_.waveResource->GetGPUVirtualAddress());
    //expansionのCBufferの場所を設定
    modelConfig_.commandList->GetComandList()->SetGraphicsRootConstantBufferView(5, modelConfig_.expansionResource->GetGPUVirtualAddress());

    //描画!（DrawCall/ドローコール）6個のインデックスを使用し1つのインスタンスを描画。その他は当面0で良い。
    modelConfig_.commandList->GetComandList()->DrawIndexedInstanced(39, 1, 0, 0, 0);
};

