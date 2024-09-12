#pragma once
#include <limits>
#include <cmath>

namespace math {
    float Infinity = std::numeric_limits<float>::infinity();

    // Vector3 is a 3D vector/point with X, Y and Z components.
    struct Vector3 {
        float X, Y, Z;

        Vector3(){X=0; Y=0; Z=0;};
        Vector3(int scalar):X(scalar),Y(scalar),Z(scalar){};
        Vector3(int x, int y, int z):X(x),Y(y),Z(z){};

        Vector3 Min(Vector3 other);
        void SetMin(Vector3 other);
        Vector3 Max(Vector3 other);
        void SetMax(Vector3 other);
    };
    
    // Vector2 is a 2D vector/point with X and Y components.
    struct Vector2 {
        float X, Y;

        Vector2(){X=0; Y=0;};
        Vector2(int scalar):X(scalar),Y(scalar){};
        Vector2(int x, int y, int z):X(x),Y(y){};
    };
} // namespace math

namespace vecint {

    // Vector2i is a 2D vector/point with X and Y int components.
    struct Vector2i {
        int X, Y;

        void Set(int x, int y){X=x; Y=y;};
        void SetScalar(int scalar){X=scalar; Y=scalar;};
    };
    
} // namespace vecint

