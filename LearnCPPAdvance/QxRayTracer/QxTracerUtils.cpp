﻿#include "QxTracerUtils.h"

void WriteColor(std::ostream& out, const Color& pixelColor)
{
    out << static_cast<int>(255.999 * pixelColor.x()) << ' '
        << static_cast<int>(255.999 * pixelColor.y()) << ' '
        << static_cast<int>(255.999 * pixelColor.z()) << '\n';
}

// 将多个sample 的结果平均写出
void WriteColor(std::ostream& out, const Color& pixelColor, int SamplesPerPixel)
{
    double r = pixelColor.x();
    double g = pixelColor.y();
    double b = pixelColor.z();

    double scale = 1.0 / SamplesPerPixel;

    // 这里的sqrt相当于pow(1/2)，进行gamma correction
    r = sqrt(scale * r);
    g = sqrt(scale * g);
    b = sqrt(scale * b);

    double tmp1 = 255.999 * r;
    double tmp2 = 255.999 * g;
    double tmp3 = 255.999 * b;

    out << static_cast<int>(tmp1) << ' '
        << static_cast<int>(tmp2) << ' '
        << static_cast<int>(tmp3) << '\n';
}