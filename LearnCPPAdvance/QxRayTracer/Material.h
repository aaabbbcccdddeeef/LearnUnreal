#pragma once
#include "Hittables.h"
#include "Ray.h"

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
    explicit  Metal(const Color& InColor)
        : Albedo(InColor)
    {
        
    }
    bool Scatter(const Ray& inRay, const HitResult& hitResult, Color& attenuation, Ray& scattered) const override;

public:
    Color Albedo;
};
