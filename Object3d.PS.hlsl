#include "object3d.hlsli"

//テクスチャを貼り付けたり、ライティングを行ったりと、もっとも主要なShaderである

struct Material
{
    float32_t4 color;
};

struct BlurParam
{
    float sigma;
    uint kernel;
};

//ConstantBufferを定義する
//ConstantBuffer<構造体>変数名 : register(b0);//配置場所
//CPUから値を渡すにはConstantBufferという機能を利用する
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<BlurParam> gSigma : register(b1);

Texture2D<float32_t4> gTexture : register(t0); //SRVはt

SamplerState gSampler : register(s0); //Samplerはs これを介してtextureを読む


float NormalizedGaussianWeight(float x, float sigma)
{  
    float k = 1.0 / (sigma * sqrt(2.0 * 3.14159265359));
    return k * exp(-(x * x) / (2.0 * sigma * sigma));
}

float4 HorizontalBlur(Texture2D sceneTex, SamplerState sample, float2 uv, float2 offset, float sigma, uint kernel)
{
    float4 color = { 0.0f, 0.0f, 0.0f, 0.0f };
    

    for (int i = -int(kernel - 1 / 2); i <= int(kernel - 1 / 2); i++)
    {
        color += sceneTex.Sample(
        sample, uv
        + float2(i * offset.x, 0)) * NormalizedGaussianWeight(i, sigma);
    }
    
    return color;
}

float4 VerticalBlur(Texture2D sceneTex, SamplerState sample, float2 uv, float2 offset, float sigma, uint kernel)
{
    float4 color = { 0.0f, 0.0f, 0.0f, 0.0f };
     
    for (int i = -int(kernel - 1 / 2); i <= int(kernel - 1 / 2); i++)  // カーネルを拡大
    {
        color += sceneTex.Sample(
        sample, uv
        + float2(0, i * offset.y)) * NormalizedGaussianWeight(i, sigma);
    }
    return color;
}

float4 GaussianBlur(Texture2D sceneTex, SamplerState sample, float2 uv, float sigma, uint kernel)
{
    float2 offset = float2(1.0 / 800.0, 1.0 / 600.0);

    float4 color = { 0.0f, 0.0f, 0.0f, 0.0f };
    
    color = HorizontalBlur(sceneTex, sample, uv, offset, sigma, kernel);
    color += VerticalBlur(sceneTex, sample, uv, offset, sigma, kernel);

    return color;
}



struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};


PixelShaderOutput main(VertexShaderOutput input)
{
 
    PixelShaderOutput output;
    
    //ガウスブラー処理をしたものを出力
    output.color = gMaterial.color * GaussianBlur(gTexture, gSampler, input.texcoord, gSigma.sigma, gSigma.kernel);

    return output;
}

