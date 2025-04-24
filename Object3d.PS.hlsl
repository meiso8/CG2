#include "object3d.hlsli"

//テクスチャを貼り付けたり、ライティングを行ったりと、もっとも主要なShaderである

struct Material
{
    float32_t4 color;
};

//ConstantBufferを定義する
//ConstantBuffer<構造体>変数名 : register(b0);//配置場所
//CPUから値を渡すにはConstantBufferという機能を利用する
ConstantBuffer<Material> gMaterial : register(b0);
//ConstantBuffer<Material> gMaterial0 : register(b0);
//ConstantBuffer<Material> gMaterial1 : register(b1);
//ConstantBuffer<Material> gMaterial2 : register(b2);
Texture2D<float32_t4> gTexture : register(t0); //SRVはt
//Texture2D<float32_t4> gTexture0 : register(t0);このような書き方が出来る
//Texture2D<float32_t4> gTexture1 : register(t1);
SamplerState gSampler : register(s0); //Samplerはs これを介してtextureを読む

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};


PixelShaderOutput main(VertexShaderOutput input)
{
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);

    PixelShaderOutput output;

    //glbal変数のgをつけている
    output.color = gMaterial.color * textureColor; //ベクトル*ベクトルと記述すると乗算が行われる
    return output;
}
