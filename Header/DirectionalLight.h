#pragma once
#include"math/Vector4.h"
#include"math/Vector3.h"

struct DirectionalLight
{
    Vector4 color;//ライトの色
    Vector3 direction;//ライトの向き　正規化すること
    float intensity;//輝度
};