#include "math.hpp"

namespace math {
    const float Infinity = std::numeric_limits<float>::infinity();
    
    // WrapMinDist returns the wrapped coordinate value that is closest to ctr
    // i.e., if going out beyond max is closer, then returns that coordinate
    // else if going below 0 is closer than not, then returns that coord
    float WrapMinDist(float ci, float max, float ctr) {
        float nwd = std::abs(ci - ctr); // no-wrap dist
        if (std::abs((ci+max)-ctr) < nwd) {
            return ci + max;
        }
        if (std::abs((ci-max)-ctr) < nwd) {
            return ci - max;
        }
        return ci;
    }
    
    // GaussVecDistNoNorm returns the gaussian of the distance between two 2D vectors
    // using given sigma standard deviation, without normalizing area under gaussian
    // (i.e., max value is 1 at dist = 0)
    float GaussVecDistNoNorm(Vec2 a, Vec2 b, float sigma) {
        float dsq = a.DistanceToSquared(b);
        return std::exp((-0.5 * dsq) / (sigma * sigma));
    }
    
    // Logistic is the logistic (sigmoid) function of x: 1/(1 + e^(-gain*(x-off)))
    float Logistic(float x, float gain, float off) {
        return 1.0 / (1.0 + std::exp(-gain*(x-off)));
    }
} // namespace math

math::Vector3::Vector3(){
    X=0;
    Y=0;
    Z=0;

}

math::Vector3::Vector3(int scalar):X(scalar),Y(scalar),Z(scalar){}

math::Vector3::Vector3(int x, int y, int z):X(x),Y(y),Z(z){}

math::Vector2::Vector2(){X=0; Y=0;}

math::Vector2::Vector2(int scalar):X(scalar),Y(scalar){}

math::Vector2::Vector2(int x, int y): X(x),Y(y) {
}

math::Vector2 math::Vector2::Sub(Vector2 other) {
    return Vector2(X-other.X, Y-other.Y);
}

math::Vector2 math::Vector2::Add(Vector2 other) {
    return Vector2(X+other.X, Y+other.Y);
}

math::Vector2 math::Vector2::Mul(Vector2 other) {
    return Vector2(X*other.X, Y*other.Y);
}

math::Vector2 math::Vector2::Div(Vector2 other) {
    return Vector2(X/other.X, Y/other.Y);
}

math::Vector2 math::Vector2::SubScalar(float scalar) {
    return Vector2(X-scalar, Y-scalar);
}

math::Vector2 math::Vector2::AddScalar(float scalar) {
    return Vector2(X+scalar, Y+scalar);
}

math::Vector2 math::Vector2::MulScalar(float scalar) {
    return Vector2(X*scalar, Y*scalar);
}

math::Vector2 math::Vector2::DivScalar(float scalar) {
    return Vector2(X/scalar, Y/scalar);
}

float math::Vector2::DistanceToSquared(Vector2 other) {
    float dx = X - other.X;
	float dy = Y - other.Y;
	return dx*dx + dy*dy;
}

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

void vecint::Vector2i::Set(int x, int y){X=x; Y=y;}

void vecint::Vector2i::SetScalar(int scalar){X=scalar; Y=scalar;}
