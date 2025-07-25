
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
    float frequency; //周期
};

struct Balloon
{
    float expansion; //膨張
    float sphere;
    float cube;
    bool isSphere;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);
ConstantBuffer<Balloon> gBalloon : register(b1);

StructuredBuffer<Wave> gWave : register(t0);


struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

float Wave(VertexShaderInput input)
{
     
    float Dot1 = dot(input.position, normalize(gWave[0].direction) * gWave[0].frequency);
    float Wave1 = cos(gWave[0].time + Dot1) * gWave[0].amplitude;
    
    float Dot2 = dot(input.position, normalize(gWave[1].direction) * gWave[1].frequency);
    float Wave2 = cos(gWave[0].time + Dot2) * gWave[1].amplitude; //1と同じ時間を入れる
    
    return Wave1 + Wave2;
}

float32_t3 Balloon(VertexShaderInput input)
{
    float32_t3 output;
    
    //法線*膨張率
    output = input.normal * gBalloon.expansion;
    return output;
}

float32_t4 Sphere(VertexShaderInput input)
{
    float32_t4 output;
    
    output.xyz = lerp(input.position.xyz, normalize(input.position.xyz), gBalloon.sphere);
    output.w = input.position.w;
    
    return output;
}

float32_t4 Cube(VertexShaderInput input)
{
    float32_t4 output;
    
    output.xyz = lerp(input.position.xyz, clamp(normalize(input.position.xyz), -0.5f, 0.5f), gBalloon.cube);
    output.w = input.position.w;
    
    return output;
}


VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    //行列の積を計算する関数がhlslの組み込み関数で定義されている
    input.position.y += Wave(input);
    
    
    input.position.xyz += Balloon(input);
    //output.position = ;
    
    if (gBalloon.isSphere)
    {
        output.position = mul(Sphere(input), gTransformationMatrix.WVP);
    }
    else
    {
        output.position = mul(Cube(input), gTransformationMatrix.WVP);
    }

    
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float32_t3x3) gTransformationMatrix.World));
    return output;
}