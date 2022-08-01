#include "Hittables.h"

bool Sphere::Hit(const Ray& InRay, double tMin, double tMax, HitResult& OutHitRes) const
{
    Vec3 oc = InRay.GetOrigin() - Center;
    auto a = InRay.GetDirection().LengthSquared();
    auto half_b = dot(oc, InRay.GetDirection());
    auto c = oc.LengthSquared() - Radius*Radius;

    auto discriminant = half_b*half_b - a*c;
    if (discriminant < 0) return false;

    double sqrtd = sqrt(discriminant);

    // find the nearest root lies in the acceptable range
    // 测试二次函数的2个解是否在规定范围内
    double root = (-half_b - sqrtd) / a;
    if (root < tMin || root > tMax)
    {
        root = (-half_b + sqrtd) / a;
        if ((root < tMin || root > tMax))
        {
            return  false;
        }
    };

    OutHitRes.t = root;
    OutHitRes.hitPoint = InRay.At(root);
    Vec3 outwardNorml = (OutHitRes.hitPoint - Center) / Radius;
    OutHitRes.SetFaceNormal(InRay, outwardNorml);
    // OutHitRes.Normal = (OutHitRes.hitPoint - Center) / Radius;
    OutHitRes.MatPtr = MatPtr;
    return  true;
}

bool HittableList::Hit(const Ray& InRay, double tMin, double tMax, HitResult& OutHitRes) const
{
    HitResult tmpRes;
    bool hitAnything = false;
    double closeToFar = tMax;

    for (const auto& object : objects)
    {
        // 注意hit之后更新max上限
        if (object->Hit(InRay, tMin, closeToFar, tmpRes))
        {
            hitAnything = true;
            closeToFar = tmpRes.t;
            OutHitRes = tmpRes;
            // break;
        }
    }

    return  hitAnything;
}
