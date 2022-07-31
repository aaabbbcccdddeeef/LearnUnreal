#pragma once
#include "Ray.h"
#include "Vec3.h"

class Camera
{
public:
    Camera()
    {
        double aspectRatio = 16.0 / 9.0;
        double viewportHeight = 2.0;
        double viewportWidth = aspectRatio * viewportHeight;
        double focalLength = 1.0;

        Origin = Vec3(0, 0, 0);
        Horizontal = Vec3(viewportWidth, 0, 0);
        Vertical = Vec3(0, viewportHeight, 0);
        LowerLeftCorner = Origin - Horizontal / 2 - Vertical / 2 - Vec3(0, 0, focalLength);
    }

    // 根据传入的uv构造相机原点指向该uv的ray
    Ray GetRay(double u, double v) const
    {
        return Ray(Origin,  LowerLeftCorner + u*Horizontal + v*Vertical - Origin);
    }
    
private:
    Point3 Origin;
    Point3 LowerLeftCorner;
    Vec3 Horizontal;
    Vec3 Vertical;
};
