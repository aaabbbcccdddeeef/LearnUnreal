#include "QxTracerUtils.h"

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
    r *= scale;
    g *= scale;
    b *= scale;

    out << static_cast<int>(255.999 * r) << ' '
        << static_cast<int>(255.999 * g) << ' '
        << static_cast<int>(255.999 * b) << '\n';
}