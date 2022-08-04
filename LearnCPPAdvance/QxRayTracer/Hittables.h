#pragma once
#include "Ray.h"
#include "Vec3.h"
#include <vector>

#include "AABB.h"

class Material;

struct HitResult
{
    Point3 hitPoint;
    Vec3 Normal;
    double t; // t表示在ray 的表达式中的t

    bool bFrontFace; // 逆时针为front face

    std::shared_ptr<Material> MatPtr;

    inline void SetFaceNormal(const Ray& inRay, const Vec3& outwardNormal)
    {
        bFrontFace = dot(inRay.GetDirection(), outwardNormal) < 0;
        Normal = bFrontFace ? outwardNormal : -outwardNormal;
    }
};

class Hittable
{
public:
    virtual bool Hit(const Ray& InRay,
        double tMin,
        double tMax,
        HitResult& OutHitRes) const = 0;

    virtual bool GetBoundingBox(double inTime0, double inTime1, AABB& outAABB) = 0;
};

class Sphere : public Hittable
{
public:
    Sphere()
    {
    }

    Sphere(const Point3& InCenter, double InRadius,
        std::shared_ptr<Material> InMaterial)
        : Center(InCenter), Radius(InRadius), MatPtr(InMaterial)
    {
    }

    bool Hit(const Ray& InRay, double tMin, double tMax, HitResult& OutHitRes) const override;
    bool GetBoundingBox(double inTime0, double inTime1, AABB& outAABB) override;

    Point3 Center;
    double Radius;

    std::shared_ptr<Material> MatPtr;
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
    bool GetBoundingBox(double inTime0, double inTime1, AABB& outAABB) override;


    void Clear() {objects.clear();}
    void Add(std::shared_ptr<Hittable> object)
    {
        objects.push_back(object);
    }

    std::vector<std::shared_ptr<Hittable>> objects;
};


class MovingSphere : public Hittable
{
public:
    MovingSphere() {  }
    MovingSphere(Point3 inCenter0, Point3 inCenter1,
        double inTime0, double inTime1,
        double inRadius, std::shared_ptr<Material> inMaterial)
            : Center0(inCenter0), Center1(inCenter1),
            Time0(inTime0), Time1(inTime1), Radius(inRadius), MatPtr(inMaterial)
    {
        
    }
    
    bool Hit(const Ray& InRay, double tMin, double tMax, HitResult& OutHitRes) const override;
    bool GetBoundingBox(double inTime0, double inTime1, AABB& outAABB) override;

    Point3 Center(double time) const;
    Point3 Center0;
    Point3 Center1;

    double Time0;
    double Time1;
    double Radius;
    std::shared_ptr<Material> MatPtr;
};


