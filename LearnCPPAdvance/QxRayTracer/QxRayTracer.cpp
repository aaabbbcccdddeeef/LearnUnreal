// QxRayTracer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "Camera.h"
#include "Hittables.h"
#include "Material.h"
#include "QxTracerUtils.h"
#include "Ray.h"
#include "Vec3.h"

Vec3 RandomInHemiSphere(const Vec3& normal)
{
    Vec3 randomVec = Vec3::RandomInUnitSphere();
    if (dot(randomVec, normal) > 0.0)
    {
        return randomVec;
    }
    else
    {
        return -randomVec;
    }
}

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

Color RayColor(const Ray& inRay, const Hittable& world, int depth)
{
    if (depth <= 0)
    {
        return Color(0, 0, 0);
    }
    
    HitResult hitRes;
    if (world.Hit(inRay, 0.001, Infinity, hitRes))
    {
        //#TODO 这里两个unit vector选择上的区别？？？？？？？
        // 这里target 位置的选择已知
        // 1. RnadomInUnitSphere 2. RandomUnitVector
        // 3. RandomInHemiSphere
        //Point3 target = hitRes.hitPoint + hitRes.Normal + Vec3::RandomUnitVector(); //Vec3::RandomInUnitSphere();
        Ray rayScattered;
        Color attenuation;
        if (hitRes.MatPtr->Scatter(
            inRay, hitRes, attenuation, rayScattered))
        {
            return  attenuation * RayColor(rayScattered, world, depth - 1);       
        }
        return  Color(0, 0, 0);
        // Point3 target = hitRes.hitPoint + RandomInHemiSphere(hitRes.Normal);
        // return  0.5 * (hitRes.Normal + Vec3(1, 1, 1));
        // return 0.5 * RayColor(Ray(hitRes.hitPoint, target - hitRes.hitPoint), world, depth - 1);
    }
    Vec3 unitDir = Unit_Vector(inRay.Dir);
    double t = 0.5 *(unitDir.y() + 1.0);
    return  (1.0-t)*Color(1.0, 1.0, 1.0) + t*Color(0.5, 0.7, 1.0);
}

void MakeTestScene1(HittableList& world)
{
    // world.Add(make_shared<Sphere>(Point3(0, 0, -1), 0.5));
    // world.Add(make_shared<Sphere>(Point3(0, -100.5, -1), 100));
    auto materialGround = std::make_shared<Lambertian>(Color(0.8, 0.8, 0.0));
    auto materialCenter = std::make_shared<Lambertian>(Color(0.1, 0.2, 0.5));
    // auto materialCenter = std::make_shared<Dielectric>(1.5);
    auto materialLeft = std::make_shared<Dielectric>(1.5);
    // auto materialLeft = std::make_shared<Metal>(Color(0.8, 0.8, 0.8), 0.3);
    auto materialRight = std::make_shared<Metal>(Color(0.8, 0.6, 0.2), 0.0);

    world.Add(make_shared<Sphere>(Point3(0.0, -100.5, -1.0), 100.0, materialGround));
    world.Add(make_shared<Sphere>(Point3(0.0, 0.0, -1.0), 0.5, materialCenter));
    world.Add(make_shared<Sphere>(Point3(-1.0, 0.0, -1.0), 0.5, materialLeft));
    // world.Add(make_shared<Sphere>(Point3(-1.0, 0.0, 1.0), -0.4, materialLeft));
    world.Add(make_shared<Sphere>(Point3(1.0, 0.0, -1.0), 0.5, materialRight));
}

void MakeTestScene2(HittableList& world)
{
    auto material_ground = make_shared<Lambertian>(Color(0.8, 0.8, 0.0));
    auto material_center = make_shared<Lambertian>(Color(0.1, 0.2, 0.5));
    auto material_left   = make_shared<Dielectric>(1.5);
    auto material_right  = make_shared<Metal>(Color(0.8, 0.6, 0.2), 0.0);

    world.Add(make_shared<Sphere>(Point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    world.Add(make_shared<Sphere>(Point3( 0.0,    0.0, -1.0),   0.5, material_center));
    world.Add(make_shared<Sphere>(Point3(-1.0,    0.0, -1.0),   0.5, material_left));
    world.Add(make_shared<Sphere>(Point3(-1.0,    0.0, -1.0), -0.45, material_left));
    world.Add(make_shared<Sphere>(Point3( 1.0,    0.0, -1.0),   0.5, material_right));
}

int main()
{
    // image
    const double aspectRatio = 16.0 / 9.0;
    const int image_width = 400;
    const int image_height = static_cast<int>(image_width / aspectRatio);
    const int SamplersPerPixel = 100;
    const int maxDepth = 50;

    // World
    auto R = cos(PI / 4);
    HittableList world;
    // MakeTestScene1(world);

    // auto materialLeft = make_shared<Lambertian>(Color(0, 0, 1));
    // auto materialRight = make_shared<Lambertian>(Color(1, 0, 0));
    //
    // world.Add(make_shared<Sphere>(Point3(-R, 0, -1), R, materialLeft));
    // world.Add(make_shared<Sphere>(Point3(R, 0, -1), R, materialRight));

    MakeTestScene2(world);
    // Camera cam(Point3(-2,2,1), Point3(0,0,-1), Vec3(0,1,0), 90, aspectRatio);
    // Camera
    // Camera cam(Point3(-2, 2, 1),
    //     Point3(0, 0, -1),
    //     Vec3(0, 1, 0),
    //     90.0, aspectRatio);
    Camera cam(Point3(-2,2,1), Point3(0,0,-1), Vec3(0,1,0), 20, aspectRatio);

    // Render
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = image_height - 1; j >= 0; --j)
    {
        std::cerr << "\rScanlines remaining" << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i)
        {
            Color pixelColor(0, 0, 0);            
            for (int s = 0; s < SamplersPerPixel; ++s)
            {
                double u = (i + RandomDouble()) / (image_width - 1);
                double v = (j + RandomDouble()) / (image_height - 1);
                // double u = (double)(i ) / (image_width - 1);
                // double v = (double)(j ) / (image_height - 1);

                Ray r = cam.GetRay(u, v);
                // pixelColor += RayColor(r, world);
                pixelColor += RayColor(r, world, maxDepth);
            }
            // Ray r(origin, lowerLeftCorner + u * horizontal + v * vertical - origin);

            // Color pixelColor = RayColor(r);
            // Color pixelColor(double(i) / (image_width - 1),
                // double(j) /( image_height - 1), 0.25);
            // Color pixelColor = RayColor(r, world);
            
            WriteColor(std::cout, pixelColor, SamplersPerPixel);
        }
    }

    std::cerr << "\nDone.\n";
}

