#pragma once

struct Vector2 {
    float x, y;

    //複合代入演算子Compound Assignement Operator
    Vector2& operator+=(float s) { x += s; y += s; return *this; }
    Vector2& operator-=(const Vector2& v) { x -= v.x, y -= v.y; return *this; }
    Vector2& operator+=(const Vector2& v) { x += v.x, y += v.y; return *this; }
    Vector2& operator/=(float s) { x /= s, y /= s; return*this; }

};