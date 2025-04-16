//float4 main( float4 pos : POSITION ) : SV_POSITION
//{
//	return pos;
//}

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderInput output;
    output.position = input.position;
    return output;
}