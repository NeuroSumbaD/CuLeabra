#pragma once
#include <limits>
#include <cmath>

namespace math {
    extern const float Infinity;// = std::numeric_limits<float>::infinity();

    // Vector3 is a 3D vector/point with X, Y and Z components.
    struct Vector3 {
        float X, Y, Z;

        Vector3();
        Vector3(int scalar);
        Vector3(int x, int y, int z);

        Vector3 Min(Vector3 other);
        void SetMin(Vector3 other);
        Vector3 Max(Vector3 other);
        void SetMax(Vector3 other);
    };
    
    // Vector2 is a 2D vector/point with X and Y components.
    struct Vector2 {
        float X, Y;

        Vector2();
        Vector2(int scalar);
        Vector2(int x, int y);

        Vector2 MulScalar(float s);
    };
} // namespace math

namespace vecint {

    // Vector2i is a 2D vector/point with X and Y int components.
    struct Vector2i {
        int X, Y;

        void Set(int x, int y);
        void SetScalar(int scalar);
    };
    
} // namespace vecint

