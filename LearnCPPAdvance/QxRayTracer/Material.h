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
        Vec3 refracted = Refract(unitDir, hitResult.Normal, refractRatio);

        scattered = Ray(hitResult.hitPoint, refracted);
        return true;
    };

public:
    double IndexOfRefraction;
};
