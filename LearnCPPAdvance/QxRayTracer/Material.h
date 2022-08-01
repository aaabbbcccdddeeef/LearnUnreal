#pragma once
#include "Hittables.h"
#include "QxTracerUtils.h"
#include "Ray.h"

struct HitResult;

class Material
{
public:
    virtual bool Scatter(
        const Ray& inRay, const HitResult& hitResult,
        Color& attenuation, Ray& scattered) const = 0;
};


class Lambertian : public  Material
{
public:
    explicit  Lambertian(const Color& InColor)
        : Albedo(InColor)
    {
        
    }
    bool Scatter(const Ray& inRay, const HitResult& hitResult, Color& attenuation, Ray& scattered)  const override;

public:
    Color Albedo;
};

class Metal : public  Material
{
public:
    explicit  Metal(const Color& InColor, double InFuzz)
        : Albedo(InColor), Fuzz(InFuzz < 1 ? InFuzz : 1)
    {
        
    }
    bool Scatter(const Ray& inRay, const HitResult& hitResult, Color& attenuation, Ray& scattered) const override;

public:
    Color Albedo;
    double Fuzz;
};


class Dielectric : public Material
{
public:
    Dielectric(double IOR)
        : IndexOfRefraction(IOR)
    {
    }
    bool Scatter(const Ray& inRay, const HitResult& hitResult, Color& attenuation, Ray& scattered) const override
    {
        attenuation = Color(1.0, 1.0, 1.0);
        double refractRatio = hitResult.bFrontFace ? (1.0/IndexOfRefraction) : IndexOfRefraction;

        Vec3 unitDir = Unit_Vector(inRay.GetDirection());
        double cosTheta = fmin(dot(-unitDir, hitResult.Normal), 1.0);
        double sinTheta = sqrt(1.0 - cosTheta * cosTheta);

        // 这种情况只有反射没有折射，称为total inner reflection
        bool bCannotRefract = refractRatio * sinTheta > 1.0;
        Vec3 direction;
        if (bCannotRefract || Reflectance(cosTheta, refractRatio) > RandomDouble())
        // if (bCannotRefract )
        {
            direction = Reflect(unitDir, hitResult.Normal);
        }   
        else
        {
            direction = Refract(unitDir, hitResult.Normal, refractRatio);
        }
        
        
        // Vec3 refracted = Refract(unitDir, hitResult.Normal, refractRatio);

        scattered = Ray(hitResult.hitPoint, direction);
        return true;
    };

private:
    static double Reflectance(double cosine, double ref_idx)
    {
        // use schlick's approximation for reflectanc
        auto r0 = (1- ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1- r0) * pow((1- cosine), 5);
    }
    
public:
    double IndexOfRefraction;
};
