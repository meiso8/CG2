#pragma once

struct Vector3 {
    float x, y, z;

    const Vector3 operator=(const Vector3& other) {
        return { x = other.x,y = other.y,z = other.z };
    }


    const Vector3 operator+=(const Vector3& other) {
        return { x += other.x,y += other.y,z += other.z };
    }

};