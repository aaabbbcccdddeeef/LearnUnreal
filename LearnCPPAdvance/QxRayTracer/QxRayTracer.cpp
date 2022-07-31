// QxRayTracer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "QxTracerUtils.h"
#include "Ray.h"
#include "Vec3.h"

Color RayColor(const Ray& InRay)
{
    Vec3 unitDir = Unit_Vector(InRay.GeDirection());
    double t = 0.5 * (unitDir.y() + 1.0 );
    return (1.0 - t) * Color(1.0, 1.0, 1.0) +
        t * Color(0.5, 0.7, 1.0);   
}

int main()
{
    // image
    const double aspectRatio = 16.0 / 9.0;
    const int image_width = 400;
    const int image_height = static_cast<int>(image_width / aspectRatio);

    // Camera
    double viewportHeight = 2.0;
    double viewportWidth = aspectRatio * viewportHeight;
    double focalLength = 1.0;

    const Vec3 origin = Vec3(0, 0, 0);
    const Vec3 horizontal = Vec3(viewportWidth, 0, 0);
    const Vec3 vertical = Vec3(0, viewportHeight, 0);
    const Vec3 lowerLeftCorner = origin - horizontal / 2 - vertical / 2 - Vec3(0, 0, focalLength);

    // Render
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = image_height - 1; j >= 0; --j)
    {
        std::cerr << "\rScanlines remaining" << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i)
        {
            double u = double(i) / (image_width - 1);
            double v = double(j) / (image_height - 1);

            Ray r(origin, lowerLeftCorner + u * horizontal + v * vertical - origin);

            Color pixelColor = RayColor(r);
            // Color pixelColor(double(i) / (image_width - 1),
                // double(j) /( image_height - 1), 0.25);
            WriteColor(std::cout, pixelColor);
        }
    }

    std::cerr << "\nDone.\n";
}

