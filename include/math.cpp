#include "math.hpp"

// Min returns min of this vector components vs. other vector.
math::Vector3 math::Vector3::Min(Vector3 other) {
    return Vector3(std::min(X, other.X), std::min(Y, other.Y), std::min(Z, other.Z));
}

// SetMin sets this vector components to the minimum values of itself and other vector.
void math::Vector3::SetMin(Vector3 other) {
    X = std::min(X, other.X);
	Y = std::min(Y, other.Y);
	Z = std::min(Z, other.Z);
}

// Max returns max of this vector components vs. other vector.
math::Vector3 math::Vector3::Max(Vector3 other) {
    return Vector3(std::max(X, other.X), std::max(Y, other.Y), std::max(Z, other.Z));
}

// SetMax sets this vector components to the maximum value of itself and other vector.
void math::Vector3::SetMax(Vector3 other) {
    X = std::max(X, other.X);
	Y = std::max(Y, other.Y);
	Z = std::max(Z, other.Z);
}