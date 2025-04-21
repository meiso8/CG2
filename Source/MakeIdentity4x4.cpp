#include "../Header/MakeIdentity4x4.h"

Matrix4x4 MakeIdentity4x4() {

    Matrix4x4 result;

    //アルゴリズムで行うよりベタ打ちの方が処理が早い？
    //for (int row = 0; row < 4; ++row) {
    //    for (int column = 0; column < 4; ++column) {

    //        if (row = column) {
    //            result.m[row][column] = 1.0f;
    //        } else {
    //            result.m[row][column] = 0.0f;
    //        }
    //    }
    //}

    result = { 1.0f,0.0f,0.0f,0.0f,
               0.0f,1.0f,0.0f,0.0f,
               0.0f,0.0f,1.0f,0.0f,
               0.0f,0.0f,0.0f,1.0f,
    };

    return result;

};