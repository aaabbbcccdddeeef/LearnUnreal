// QxRayTracer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "Hittables.h"
#include "QxTracerUtils.h"
#include "Ray.h"
#include "Vec3.h"

// simple fied version
double HitSphere(const Point3& center, double radius, const Ray& ray)
{
    Vec3 oc = ray.Origin - center;
    double a = ray.GetDirection().LengthSquared();
    double half_b = dot(oc, ray.GetDirection());
    double c = oc.LengthSquared() - radius * radius;

    double discriminant = half_b * half_b - a * c;
    // return (discriminant > 0);
    if (discriminant < 0)
    {
        return -1.0;
    }
    else
    {
        return (-half_b - sqrt(discriminant)) / a;
    }
}

double HitSphereComplete(const Point3& center, double radius, const Ray& ray)
{
    Vec3 oc = ray.Origin - center;
    double a = dot(ray.GetDirection(), ray.GetDirection());
    double b = 2.0 * dot(oc, ray.GetDirection());
    double c = dot(oc, oc) - radius * radius;

    double discriminant = b * b - 4 * a * c;
    // return (discriminant > 0);
    if (discriminant < 0)
    {
        return -1.0;
    }
    else
    {
        return (-b - sqrt(discriminant)) / (2.0 * a);
    }
}


Color RayColor(const Ray& InRay)
{
    // if (HitSphere(Point3(0, 0, -1), 0.5, InRay))
    // {
    //     return Color(1, 0, 0);
    // }
    double hitResult = HitSphere(Point3(0, 0, -1), 0.5, InRay);
    if (hitResult > 0.0)
    {
        Vec3 normal = Unit_Vector(InRay.At(hitResult) - Point3(0, 0, -1));
        return 0.5 * (normal + Vec3(1., 1., 1.));
    }
    
    Vec3 unitDir = Unit_Vector(InRay.GetDirection());
    double t = 0.5 * (unitDir.y() + 1.0 );
    return (1.0 - t) * Color(1.0, 1.0, 1.0) +
        t * Color(0.5, 0.7, 1.0);   
}

Color RayColor(const Ray& inRay, const Hittable& world)
{
    HitResult hitRes;
    if (world.Hit(inRay, 0, Infinity, hitRes))
    {
        return  0.5 * (hitRes.Normal + Vec3(1, 1, 1));
    }
    Vec3 unitDir = Unit_Vector(inRay.Dir);
    double t = 0.5 *(unitDir.y() + 1.0);
    return  (1.0-t)*Color(1.0, 1.0, 1.0) + t*Color(0.5, 0.7, 1.0);
}

int main()
{
    // image
    const double aspectRatio = 16.0 / 9.0;
    const int image_width = 400;
    const int image_height = static_cast<int>(image_width / aspectRatio);

    // World
    HittableList world;
    world.Add(make_shared<Sphere>(Point3(0, 0, -1), 0.5));
    world.Add(make_shared<Sphere>(Point3(0, -100.5, -1), 100));
    
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

            // Color pixelColor = RayColor(r);
            // Color pixelColor(double(i) / (image_width - 1),
                // double(j) /( image_height - 1), 0.25);
            Color pixelColor = RayColor(r, world);
            WriteColor(std::cout, pixelColor);
        }
    }

    std::cerr << "\nDone.\n";
}

