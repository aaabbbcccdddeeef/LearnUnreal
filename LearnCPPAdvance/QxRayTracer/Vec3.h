#pragma once
#include <cmath>
#include <iostream>


using std::sqrt;

class Vec3
{
public:
    Vec3()
        : e{0, 0, 0}
    {
    }
    Vec3(double e0, double e1, double e2)
        : e{e0, e1, e2}
    {
    }

    double x() const {return e[0];}
    double y() const {return e[1];}
    double z() const {return e[2];}

    Vec3 operator-() const
    {
        return Vec3(-e[0], -e[1], -e[2]);
    }

    double operator[](int i) const
    {
        return e[i];
    }

    double& operator[](int i)
    {
        return e[i];
    }

    Vec3& operator+=(const Vec3& v)
    {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }

    Vec3& operator*=(const double t)
    {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    Vec3& operator/=(const double t)
    {
        return *this *= 1/t;
    }

    double LengthSquared() const
    {
        return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
    }
    
    double Length() const
    {
        return sqrt(LengthSquared());
    }

    bool IsNearZero() const
    {
        const auto s = 1e-8;
        return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
    }

    static Vec3 Random();

    static Vec3 Random(double min, double max);

    static Vec3 RandomInUnitSphere();

    static Vec3 RandomUnitVector();

    const static  Vec3 White ;
public:
    double e[3];
};

using Point3 = Vec3;
using Color = Vec3;




// vec3 Utility Functions
inline std::ostream& operator<<(std::ostream& out, const Vec3 &v)
{
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline Vec3 operator+(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3(lhs.e[0] + rhs.e[0], lhs.e[1] + rhs.e[1], lhs.e[2] + rhs.e[2]);
}

inline Vec3 operator-(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3(lhs.e[0] - rhs.e[0], lhs.e[1] - rhs.e[1], lhs.e[2] - rhs.e[2]);
}

inline Vec3 operator*(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3(lhs.e[0] * rhs.e[0], lhs.e[1] * rhs.e[1], lhs.e[2] * rhs.e[2]);
}

inline Vec3 operator*(double t, const Vec3& v)
{
    return  Vec3(v.e[0] * t, v.e[1] * t, v.e[2] * t);
}

inline Vec3 operator*(const Vec3&v, double t)
{
    return t * v;
}

inline Vec3 operator/(const Vec3& v, double t)
{
    return (1.0 / t) * v;
}

inline double dot(const Vec3& u, const Vec3& v)
{
    return  u.e[0] * v.e[0] +
        u.e[1] * v.e[1] + u.e[2] * v.e[2];
}

inline Vec3 cross(const Vec3& u, const Vec3& v)
{
    return Vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
               u.e[2] * v.e[0] - u.e[0] * v.e[2],
               u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline Vec3 Unit_Vector(const Vec3& v)
{
    return v / v.Length();
}