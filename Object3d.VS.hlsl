//float4 main( float4 pos : POSITION ) : SV_POSITION
//{
//	return pos;
//}

//struct Matrix4x4
//{
//float4x4
//};

struct TransformationMatrix
{
    float32_t4x4 WVP;
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);
struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
};

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    //行列の積を計算する関数がhlslの組み込み関数で定義されている
    //output.position = input.position;
    output.position = mul(input.position, gTransformationMatrix.WVP);
    return output;
}