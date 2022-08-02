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
        double aspectRatio,
        double Aperture,
        double focusDist,
        double inTime0,
        double intTime1
        )
    {
        // double aspectRatio = 16.0 / 9.0;
        double theta = DegreesToRadians(vFOV);
        double h = tan(theta / 2);
        double viewportHeight = 2.0 * h;
        double viewportWidth = aspectRatio * viewportHeight;
        double focalLength = 1.0;

        // 注意：当前使用的坐标系是z指向屏幕外面,yup的右手系,相机看向-z,所以下面这样算
        w = Unit_Vector(inPosition - lookAt);
         u = Unit_Vector(cross(vUp, w));
         v = cross(w, u);
        
        Origin = inPosition;
        Horizontal =focusDist * viewportWidth * u;
        Vertical = focusDist *  viewportHeight * v;
        LowerLeftCorner = Origin - Horizontal / 2 - Vertical / 2 - focusDist * w;

        LensRadius = Aperture / 2;

        Time0 = inTime0;
        Time1 = intTime1;
    }

    // 根据传入的uv构造相机原点指向该uv的ray
    Ray GetRay(double s,double t) const
    {
        // return Ray(Origin,  LowerLeftCorner + u*Horizontal + v*Vertical - Origin);
        // return Ray(Origin, LowerLeftCorner + u * Horizontal + v * Vertical - Origin);
        Vec3 rd = LensRadius * RandomInUnitDisk();
        Vec3 offset = u * rd.x() + v * rd.y();

        return Ray(
                Origin + offset,
                LowerLeftCorner + s * Horizontal + t * Vertical - Origin - offset,
                RandomDouble(Time0, Time1)
            );
    }
    
private:
    Point3 Origin;
    Point3 LowerLeftCorner;
    Vec3 Horizontal;
    Vec3 Vertical;

    Vec3 u,v,w;
    double LensRadius = 1.0;

    double Time0 = 0.;
    double Time1 = 1.;
};
