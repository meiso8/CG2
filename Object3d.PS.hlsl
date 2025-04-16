//float4 main( float4 pos : POSITION ) : SV_POSITION
//{
//	return pos;
//}

//テクスチャを貼り付けたり、ライティングを行ったりと、もっとも主要なShaderである

struct Material
{
    float4 color;
};

//ConstantBufferを定義する
//ConstantBuffer<構造体>変数名 : register(b0);//配置場所
//CPUから値を渡すにはConstantBufferという機能を利用する
ConstantBuffer<Material> gMaterial : register(b0);
struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main()
{
    PixelShaderOutput output;
    //glbal変数のgをつけている
    output.color = gMaterial.color;
    return output;
}
