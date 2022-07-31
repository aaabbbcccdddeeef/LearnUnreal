#pragma once
#include "Ray.h"
#include "Vec3.h"
#include <vector>

struct HitResult
{
    Point3 hitPoint;
    Vec3 Normal;
    double t; // t表示在ray 的表达式中的t

    bool front_face; // 逆时针为front face

    inline void SetFaceNormal(const Ray& inRay, const Vec3& outwardNormal)
    {
        front_face = dot(inRay.GetDirection(), outwardNormal) < 0;
        Normal = front_face ? outwardNormal : -outwardNormal;
    }
};

class Hittable
{
public:
    virtual bool Hit(const Ray& InRay,
        double tMin,
        double tMax,
        HitResult& OutHitRes) const = 0;
};

class Sphere : public Hittable
{
public:
    Sphere()
    {
    }

    Sphere(const Point3& InCenter, double InRadius)
        : Center(InCenter), Radius(InRadius)
    {
    }

    bool Hit(const Ray& InRay, double tMin, double tMax, HitResult& OutHitRes) const override;

    Point3 Center;
    double Radius;
};


class HittableList : public Hittable
{
public:
    HittableList() {  }
    HittableList(std::shared_ptr<Hittable> object)
    {
        Add(object);
    }
    bool Hit(const Ray& InRay, double tMin, double tMax, HitResult& OutHitRes) const override;


    void Clear() {objects.clear();}
    void Add(std::shared_ptr<Hittable> object)
    {
        objects.push_back(object);
    }
public:
    std::vector<std::shared_ptr<Hittable>> objects;
};
