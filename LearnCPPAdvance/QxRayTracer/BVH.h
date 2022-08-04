#pragma once
#include "Hittables.h"

class BVH_Node : public Hittable
{
public:
    BVH_Node() {  }
    BVH_Node(const HittableList& inHittabList, double inTime0,double inTime1)
        : BVH_Node(inHittabList.objects, 0, inHittabList.objects.size(),
            inTime0, inTime1)
    {
    }

    BVH_Node(const std::vector<std::shared_ptr<Hittable>>& srcObjects,
        size_t start, size_t end, double inTime0, double inTime1);
   

    bool Hit(const Ray& InRay, double tMin, double tMax, HitResult& OutHitRes) const override;
    bool GetBoundingBox(double inTime0, double inTime1, AABB& outAABB) override;


    std::shared_ptr<Hittable> LeftChild;
    std::shared_ptr<Hittable> RightChild;
    AABB Bounds;
};
