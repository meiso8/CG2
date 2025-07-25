#include"../../Header/math/Length.h"
#include"../../Header/math/Dot.h"
#include <cmath>

//長さ(ノルム)
float Length(const Vector3& v) {
    return { sqrtf(Dot(v,v)) };
};

float Length(const Vector2& v) {
    return { sqrtf(Dot(v,v)) };
};