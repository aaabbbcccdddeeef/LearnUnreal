﻿#pragma once

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

inline double degreesToRadians(double degrees)
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