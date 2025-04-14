//float4 main(float4 pos : POSITION) : SV_POSITION
//{
//    return pos;
//}

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    //セマンティクス(Semantics) SV = SemanticsValue

};

struct VertexShaderInput
{
    float4 position : POSITION;
    
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = input.position;
    return output;
}