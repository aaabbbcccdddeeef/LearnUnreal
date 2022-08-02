#pragma once
#include "Vec3.h"

class Ray
{
public:
    Ray() {  }

    Ray(const Point3& origin,
        const Vec3& direction,
        double  time=0.0)
            : Origin(origin), Dir(direction), tm(time)
    {
    }

    Point3 GetOrigin() const
    {
        return Origin;
    }

    Vec3 GetDirection() const
    {
        return Dir;    
    }

    double GetTime() const
    {
        return tm;
    }

    Point3 At(double t) const
    {
        return Origin + t * Dir;
    }
public:
    Point3 Origin;
    Vec3 Dir;
    double tm;
};

