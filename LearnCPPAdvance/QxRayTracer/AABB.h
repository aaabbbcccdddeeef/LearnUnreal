#pragma once
#include "Ray.h"
#include "Vec3.h"

class AABB
{
public:
    AABB() {  }
    AABB(const Point3& inMinPoint, const Point3& inMaxPoint)
        : MinPoint(inMinPoint), MaxPoint(inMaxPoint)
    {
    }

    // 优化版本的hit
    bool HitOptimized(const Ray& r, double t_min, double t_max) const
    {
        for (int a = 0; a < 3; a++) {
            auto invD = 1.0f / r.GetDirection()[a];
            auto t0 = (MinPoint[a] - r.GetOrigin()[a]) * invD;
            auto t1 = (MaxPoint[a] - r.GetOrigin()[a]) * invD;
            if (invD < 0.0f)
                std::swap(t0, t1);
            t_min = t0 > t_min ? t0 : t_min;
            t_max = t1 < t_max ? t1 : t_max;
            if (t_max <= t_min)
                return false;
        }
        return true;
    }
    
    // 这里的算法参照One weekend ray tracing 源文档,
    // https://raytracing.github.io/books/RayTracingTheNextWeek.html#boundingvolumehierarchies
    bool Hit(const Ray& inRay, double tMin, double tMax) const
    {
        // 分别计算x/y/z 3个纬度
        for (int a = 0; a < 3; ++a)
        {
            auto t0 = fmin((MinPoint[a] - inRay.Origin[a]) / inRay.GetDirection()[a],
                (MaxPoint[a] - inRay.Origin[a]) / inRay.GetDirection()[a]);
            auto t1 = fmax((MinPoint[a] - inRay.Origin[a]) / inRay.GetDirection()[a],
                (MaxPoint[a] - inRay.Origin[a]) / inRay.GetDirection()[a]);
            tMin = fmax(t0, tMin);
            tMax = fmin(t1, tMax);
            if (tMax <= tMin)
            {
                return false;
            }
        }
        return true;
    }

    Point3 GetMinPoint() const
    {
        return MinPoint;
    }
    Point3 GetMaxPoint() const
    {
        return MaxPoint;
    }

    static  AABB SurroundingBox(const AABB& box0, const AABB& box1)
    {
        Vec3 small(fmin(box0.GetMinPoint().x(), box1.GetMinPoint().x()),
               fmin(box0.GetMinPoint().y(), box1.GetMinPoint().y()),
               fmin(box0.GetMinPoint().z(), box1.GetMinPoint().z()));

        Vec3 big(fmax(box0.GetMaxPoint().x(), box1.GetMaxPoint().x()),
                   fmax(box0.GetMaxPoint().y(), box1.GetMaxPoint().y()),
                   fmax(box0.GetMaxPoint().z(), box1.GetMaxPoint().z()));

        return AABB(small,big);
    }
    
public:
    Point3 MinPoint;
    Point3 MaxPoint;
};

