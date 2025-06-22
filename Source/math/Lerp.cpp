#include "../../Header/math/Lerp.h"
#include"../../Header/math/Operator.h"

Vector3 Lerp(Vector3 start, Vector3 end, float t) {


    return { start * (1.0f - t) + end * t };



};
