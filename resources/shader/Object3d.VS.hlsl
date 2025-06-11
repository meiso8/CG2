
#include "object3d.hlsli"

struct TransformationMatrix
{
    float32_t4x4 WVP;
    float32_t4x4 World;
};

struct Wave
{
    float32_t4 direction; //方向
    float time; // アニメーション用の時間変数
    float amplitude; //振幅
    float frequency;//周期
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

//
StructuredBuffer<Wave> gWave : register(t0);


struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

float Wave(VertexShaderInput input)
{
  
    float outputY;
    
    uint num = 3;
    
 
    //float32_t4 pos = Loop(input.position, num);
    float Dot1 = dot(input.position, normalize(gWave[0].direction) * gWave[0].frequency);
    float Wave1 = sin(gWave[0].time + Dot1) * gWave[0].amplitude;

    //float Dot2 = dot(pos, normalize(gWave[1].direction));
    //float Wave2 = sin(gWave[0].time + gWave[1].time + Dot2) * gWave[1].amplitude;
    //基本の時間に差分をかける
    
    // 他の波も追加可能
    outputY = Wave1 /*+ Wave2*/; // 波を合成
    return outputY;
    
}

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    //行列の積を計算する関数がhlslの組み込み関数で定義されている
    
    input.position.y += Wave(input);

    output.position = mul(input.position, gTransformationMatrix.WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float32_t3x3) gTransformationMatrix.World));
    return output;
}