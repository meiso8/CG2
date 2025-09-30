#pragma once
#include"Vector3.h"

struct Wave
{
    Vector3 direction;
    float padding;
    float time; // アニメーション用の時間変数
    float amplitude; //振幅
    float frequency;//周期
};

