#include "object3d.hlsli"
#include"DirectionalLight.hlsli"

//テクスチャを貼り付けたり、ライティングを行ったりと、もっとも主要なShaderである


struct Material
{
    float32_t4 color;
    int32_t enableLighting;

};

//ConstantBufferを定義する
//ConstantBuffer<構造体>変数名 : register(b0);//配置場所
//CPUから値を渡すにはConstantBufferという機能を利用する
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

Texture2D<float32_t4> gTexture : register(t0); //SRVはt
SamplerState gSampler : register(s0); //Samplerはs これを介してtextureを読む

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};


PixelShaderOutput main(VertexShaderOutput input)
{
 
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);

    PixelShaderOutput output;

    if (gMaterial.enableLighting != 0){
    //Lightingする場合
        float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
    }else{
        //Lightingしない場合。前回までと同じ演算
        output.color = gMaterial.color * textureColor; //ベクトル*ベクトルと記述すると乗算が行われる
    
    }
    
    return output;
}
