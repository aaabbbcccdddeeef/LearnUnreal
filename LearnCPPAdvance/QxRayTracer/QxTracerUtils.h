#pragma once

#include <cmath>
#include <limits>
#include <memory>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <random>

#include "Vec3.h"

using  std::shared_ptr;
using  std::make_shared;
using std::sqrt;

void WriteColor(std::ostream& out, const Color& pixelColor);
void WriteColor(std::ostream& out, const Color& pixelColor, int SamplesPerPixel);

const double Infinity = std::numeric_limits<double>::infinity();
const double PI = 3.1415926535897932385;

inline double DegreesToRadians(double degrees)
{
    return degrees * PI / 180.0;
}

// 返回[0, 1) 的随机数
inline  double RandomDouble()
{
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    double tmp = distribution(generator);
    return tmp;
}

// 返回[min, max)的随机数
inline double RandomDouble(double min, double max)
{
    return min + RandomDouble() * (max - min);
}

inline double clamp(double x, double min, double max)
{
    if (x > max)
    {
        return  max;
    }
    if (x < min)
    {
        return min;
    }
    return x;
}

inline Vec3 Reflect(const Vec3& v, const Vec3& n)
{
    return v - 2 * dot(v, n) * n;
}

// 这段的由来参考文档
inline Vec3 Refract(const Vec3& incidentVec, const Vec3& normal, double etai_over_etat)
{
    auto cosTheta = fmin(dot(-incidentVec, normal), 1.0);
    Vec3 r_out_perp = etai_over_etat * (incidentVec + cosTheta * normal);
    Vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.LengthSquared())) * normal;
    return r_out_perp + r_out_parallel;
}