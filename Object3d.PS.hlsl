//float4 main( float4 pos : POSITION ) : SV_POSITION
//{
//	return pos;
//}

//テクスチャを貼り付けたり、ライティングを行ったりと、もっとも主要なShaderである

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main()
{
    PixelShaderOutput output;
    output.color = float4(1.0, 1.0, 1.0, 1.0);
    return output;
}
