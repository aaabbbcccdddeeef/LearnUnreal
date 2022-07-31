#include "Vec3.h"

#include "QxTracerUtils.h"

Vec3 Vec3::Random()
{
    return Vec3(RandomDouble(), RandomDouble(), RandomDouble());
}

Vec3 Vec3::Random(double min, double max)
{
    return Vec3(RandomDouble(min, max),
                RandomDouble(min, max),
                RandomDouble(min, max));
}

Vec3 Vec3::RandomInUnitSphere()
{
    while (true)
    {
        Vec3 p = Vec3::Random(-1, 1);
        if (p.LengthSquared() >= 1)
        {
            continue;
        }
        return p;
    }
}
