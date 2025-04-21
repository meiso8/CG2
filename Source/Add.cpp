#include"../Header/Add.h"

//行列の加法
Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2) {

    Matrix4x4 result;

    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            result.m[row][column] = m1.m[row][column] + m2.m[row][column];
        }
    }

    return result;

};