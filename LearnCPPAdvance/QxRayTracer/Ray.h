#pragma once
#include "Vec3.h"

class Ray
{
public:
    Ray() {  }

    Ray(const Point3& origin,
        const Vec3& direction)
            : Origin(origin), Dir(direction)
    {
    }

    Point3 GetOrigin() const
    {
        return Origin;
    }

    Vec3 GeDirection() const
    {
        return Dir;    
    }

    Point3 At(double t) const
    {
        return Origin + t * Dir;
    }
public:
    Point3 Origin;
    Vec3 Dir;
};

