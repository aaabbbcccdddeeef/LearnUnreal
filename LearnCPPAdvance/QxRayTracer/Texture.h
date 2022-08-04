#pragma once
#include "Vec3.h"

class Texture
{
public:
    virtual Color Sample(double u, double v, const Point3& p) = 0;
};

class SolideColor : public Texture
{
public:
    SolideColor() {  }
    SolideColor(const Color& inColor)
        : ColorValue(inColor)
    {
    }

    SolideColor(double r, double g, double b)
        : ColorValue(r, g, b)
    {
    }
    
    Color Sample(double u, double v, const Point3& p) override;

private:
    Color ColorValue = Color::White;
};
