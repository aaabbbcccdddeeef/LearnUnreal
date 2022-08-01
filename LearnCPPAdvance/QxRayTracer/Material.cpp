#include "Material.h"

#include "QxTracerUtils.h"

bool Lambertian::Scatter(const Ray& inRay, const HitResult& hitResult, Color& attenuation, Ray& scattered) const
{
    Vec3 scatterDirection =
        hitResult.Normal + Vec3::RandomUnitVector();

    // catch degernearte scatter direction
    if (scatterDirection.IsNearZero())
    {
        scatterDirection = hitResult.Normal;
    }
    
    scattered = Ray(hitResult.hitPoint, scatterDirection);
    attenuation = Albedo;
    return true;
}

bool Metal::Scatter(const Ray& inRay, const HitResult& hitResult, Color& attenuation, Ray& scattered) const
{
    Vec3 reflected = Reflect(Unit_Vector(inRay.GetDirection()), hitResult.Normal);
    scattered = Ray(hitResult.hitPoint, reflected + Fuzz * Vec3::RandomInUnitSphere()); //这里+随机向量是为了造成一定的反射方向上的模糊
    attenuation = Albedo;
    return (dot(scattered.GetDirection(), hitResult.Normal) > 0);
}
