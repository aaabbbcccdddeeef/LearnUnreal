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
    scattered = Ray(hitResult.hitPoint, reflected);
    attenuation = Albedo;
    return (dot(scattered.GetDirection(), hitResult.Normal) > 0);
}
