#pragma once

#include <cmath>
#include <limits>
#include <memory>
#include <iostream>
#include <vector>

#include "Vec3.h"

using  std::shared_ptr;
using  std::make_shared;
using std::sqrt;

void WriteColor(std::ostream& out, const Color& pixelColor);


const double Infinity = std::numeric_limits<double>::infinity();
const double PI = 3.1415926535897932385;

inline double degreesToRadians(double degrees)
{
    return degrees * PI / 180.0;
}