#pragma once
#include "QxTracerUtils.h"
#include "Ray.h"
#include "Vec3.h"

class Camera
{
public:
    Camera(Point3 inPosition,
        Point3 lookAt,
        Vec3 vUp,
        double vFOV,
        double aspectRatio)
    {
        // double aspectRatio = 16.0 / 9.0;
        double theta = DegreesToRadians(vFOV);
        double h = tan(theta / 2);
        double viewportHeight = 2.0 * h;
        double viewportWidth = aspectRatio * viewportHeight;
        double focalLength = 1.0;

        // 注意：当前使用的坐标系是z指向屏幕外面,yup的右手系,相机看向-z,所以下面这样算
        Vec3 w = Unit_Vector(inPosition - lookAt);
        Vec3 u = Unit_Vector(cross(vUp, w));
        Vec3 v = cross(w, u);
        
        Origin = inPosition;
        Horizontal =viewportWidth * u;
        Vertical = viewportHeight * v;
        LowerLeftCorner = Origin - Horizontal / 2 - Vertical / 2 - w;
    }

    // 根据传入的uv构造相机原点指向该uv的ray
    Ray GetRay(double u, double v) const
    {
        // return Ray(Origin,  LowerLeftCorner + u*Horizontal + v*Vertical - Origin);
        return Ray(Origin, LowerLeftCorner + u * Horizontal + v * Vertical - Origin);
    }
    
private:
    Point3 Origin;
    Point3 LowerLeftCorner;
    Vec3 Horizontal;
    Vec3 Vertical;
};
