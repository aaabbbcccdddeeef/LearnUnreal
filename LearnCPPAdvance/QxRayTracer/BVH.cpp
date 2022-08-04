#include "BVH.h"

#include "QxTracerUtils.h"

// 空间划分的核心实现部分，下面是基本实现思想
// 1. 随机选一个轴作为划分依据
// 2. 对hittables排序
// 3. 左右各放一半，递归构建左右子节点, 如果当前只剩 2个要处理的，左右各方一个结束， 剩一个则都放这一个结束
BVH_Node::BVH_Node(const std::vector<std::shared_ptr<Hittable>>& srcObjects,
    size_t start, size_t end,
    double inTime0,
    double inTime1)
{
    std::vector<std::shared_ptr<Hittable>> objects = srcObjects;

    int axis = RandomInt(0, 2);

    auto compareFunc = [axis](const std::shared_ptr<Hittable> left, const std::shared_ptr<Hittable> right)
    {
      AABB boxA;
        AABB boxB;
        if(!left->GetBoundingBox(0, 0, boxA) ||
            !right->GetBoundingBox(0, 0, boxB))
        {
            std::cerr << "No bounding box in bvh_node construct.\n";
        }
        return boxA.GetMinPoint().e[axis] < boxB.GetMinPoint().e[axis];
    };

    size_t objectSpan = end - start;

    if (objectSpan == 1)
    {
        LeftChild = RightChild = objects[start];
    }
    else if(objectSpan == 2)
    {
        if (compareFunc(objects[start], objects[start + 1]))
        {
            LeftChild = objects[start];
            RightChild = objects[start + 1];
        }
        else
        {
            LeftChild = objects[start + 1];
            RightChild = objects[start];
        }
    }
    else
    {
        std::sort(objects.begin() + start, objects.end() + end, compareFunc);

        auto mid = start + objectSpan / 2;
        LeftChild = std::make_shared<BVH_Node>(objects, start, mid, inTime0, inTime1);
        RightChild = std::make_shared<BVH_Node>(objects, mid, end, inTime0, inTime1);
    }

    AABB boxLeft,boxRight;

    if (!LeftChild->GetBoundingBox(inTime0, inTime1, boxLeft) ||
        !RightChild->GetBoundingBox(inTime0, inTime1, boxRight))
    {
        std::cerr << "No bounding box in bvh_node constructor.\n";
    }
    Bounds = AABB::SurroundingBox(boxLeft, boxRight);
}

bool BVH_Node::Hit(const Ray& InRay, double tMin, double tMax, HitResult& OutHitRes) const
{
    if (!Bounds.Hit(InRay, tMin, tMax))
    {
        return false;
    }

    bool hitLeft = LeftChild->Hit(InRay, tMin, tMax, OutHitRes);
    // 注意这里hitLeft 之后要更新 upper bounds
    bool hitRight = RightChild->Hit(InRay, tMin, hitLeft ? OutHitRes.t : tMax, OutHitRes);

    return hitLeft || hitRight;
}

bool BVH_Node::GetBoundingBox(double inTime0, double inTime1, AABB& outAABB)
{
    outAABB = Bounds;
    return true;
}
