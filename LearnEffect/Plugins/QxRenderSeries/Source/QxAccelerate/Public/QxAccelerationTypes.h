#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QxAccelerationTypes.generated.h"

struct FQxKdtreeNode
{
    // 到data 的索引
    int32 Index = -1;
    // 这个节点用哪个轴进行划分
    int32 Axis = -1;
    
    // FQxKdtreeNode* ChildLeft = nullptr;
    // FQxKdtreeNode* ChildRight = nullptr;
    // TUniquePtr<FQxKdtreeNode> ChildLeft = nullptr;
    // TUniquePtr<FQxKdtreeNode> ChildRight = nullptr;
    TSharedPtr<FQxKdtreeNode> ChildLeft = nullptr;
    TSharedPtr<FQxKdtreeNode> ChildRight = nullptr;
    ~FQxKdtreeNode()
    {
        // ChildLeft.Reset();
        // ChildRight.Reset();
    }
    // static void ClearNode(FQxKdtreeNode* InNode)
    // {
    //     if (InNode == nullptr)
    //     {
    //         return;
    //     }
    //     if (InNode->ChildLeft)
    //     {
    //         ClearNode(InNode->ChildLeft);
    //     }
    //     if (InNode->ChildRight)
    //     {
    //         ClearNode(InNode->ChildRight);
    //     }
    // }

    
};

// 这里实现的kd 树是一个最简单的，每个node只有一个data的索引
USTRUCT(BlueprintType)
struct FQxKdtree
{
    GENERATED_BODY()

    TArray<FVector> Points;
    // FQxKdtreeNode*  Root;
    // TUniquePtr<FQxKdtreeNode> Root;
    TSharedPtr<FQxKdtreeNode> Root;

public:
    void Clear()
    {
        Root.Reset();
        Points.Empty();
    }

    void BuildKdtree(const TArray<FVector>& InPoints);
    void SearchKNNPoint(const FVector& TargetPoint, int32 k, TArray<FQxKdtreeNode*>& OutNodes) ;

    static TSharedPtr<FQxKdtreeNode>  BuildNode(FQxKdtree& Tree, TArrayView<int32> IndiceSpan, int32 Depth);
    void CollectFromKdtree(const FVector& Vector, float Radius, TArray<int32>& OutIndices) const;

    FVector SearchNearestPoint(const FVector& InPoint) const
    {
        return Points[SearchNearestNode(InPoint)->Index];
    }

    void DumpKdtree();
private:
    void DumpKdtree(FQxKdtreeNode* InNode);

    void DumpNode(FQxKdtreeNode* InNode);
    
    void CollectFromKdtree(const FQxKdtreeNode* InNode, const FVector& Vector, float Radius, TArray<int32>& OutIndices) const;

    FQxKdtreeNode* SearchNearestNode(const FVector& InPoint) const;

    void SearchNearestNode(const FVector& TargetPoint, FQxKdtreeNode* InNode, FQxKdtreeNode*& ResultNode, double& MinDist) const;
};

static constexpr int32 EIGHT = 8;

// template<typename  T>
class TQxOctreeNode2 
{
public:
    TQxOctreeNode2(float InBaseLength, float InMinSize, float InLooseness, FVector InCenter)
    {
        SetValues(InBaseLength, InMinSize, InLooseness, InCenter);
    }

    // 一个节点最多添加多少物体
    static constexpr int32  NUM_OBJECTS_ALLOWED = 8;
    
    // 这两个应该可以用ue4 自带的bound 替换#TODO
    // FVector Center;
    // FVector Extend;
    // 这个node的bounds
    FBoxSphereBounds Bounds;
    float MinSize = 20; // 最小分割尺寸
    int32 MaxCount = 4;
    int32 Depth = -1;

    // 当前node 的looseness
    float Looseness = 1.f;

    // length of this node，这个节点的base extent
    float BaseExtent;

    // 这个节点最终实际的extent，考虑looseness
    float ActualExtent;

    

    // 固定有8个child ??
    // TStaticArray<TSharedPtr<TQxOctreeNode<T>>, EIGHT>  Children;
    TArray<TSharedPtr<TQxOctreeNode2>, TInlineAllocator<8>> Children;

    bool  HasChildren() const
    {
        return bHasChildren;
    }

    // children的潜在bounds，这个是最终实际用的bounds, 考虑了looseness 不是base bounds
    // #TODO 这里换成optional 以节省内存
    // TStaticArray<FBoxSphereBounds, EIGHT> ChildBounds;
    TArray<FBoxSphereBounds, TInlineAllocator<8>> ChildBounds;


    // octree中的一个物体
    struct FOctreeObject
    {
        // TSharedPtr<T> Object;
        // T Object;
        AActor* Object;
        FBoxSphereBounds Bounds;
    };

    // 这个节点引用的物体s
    // TArray<TSharedPtr<T>> Objects;
    TArray<FOctreeObject> Objects;

    void SetValues(float InLength, float InMinSize, float InLooseness, FVector InCenter)
    {
        BaseExtent = InLength;
        MinSize = InMinSize;
        Looseness = InLooseness;
        ActualExtent = Looseness * BaseExtent;
        Bounds.Origin = InCenter;
        Bounds.BoxExtent = FVector::OneVector * ActualExtent;

        // 更新设置child bounds
        const float ChildBaseExtent = BaseExtent / 2.f;
        const float ChildActualExtent = ChildBaseExtent * Looseness;
        bHasChildren = true;
        static FVector BoxVertex[] =
            {
                FVector(-1, 1, -1),
                FVector(1, 1, -1),
                FVector(-1, 1, 1),
                FVector(1, 1, 1),
                FVector(-1, -1, -1),
                FVector(1, -1, -1),
                FVector(-1, -1, 1),
                FVector(1, -1, 1),
            };
        // Test
        static_assert(UE_ARRAY_COUNT(BoxVertex) == 8);
        ChildBounds.Reset(8);
        for (int32 i = 0; i < EIGHT; ++i)
        {
            auto tmp = FBoxSphereBounds(
                Bounds.Origin + BoxVertex[i] * ChildBaseExtent,
                FVector::OneVector * ChildActualExtent,
                ChildActualExtent * 2 // 这里先这样做
                );
            ChildBounds.Add(tmp);
        }
    }


    void SubAdd(AActor* InObj, FBoxSphereBounds InBounds)
    {
        if (!bHasChildren)
        {
            // Just add if few objects are here, or children would be below min size
            if (Objects.Num() < NUM_OBJECTS_ALLOWED ||
                BaseExtent < MinSize) 
            {
                FOctreeObject OctreeObject{InObj, InBounds};
                Objects.Add(OctreeObject);
                return;
            }

            // int32 BestFitChild;
            if (Children.Num() == 0)
            {
                // Split();
            }
        }
    }
    
    /**
     * @brief 
     * @param InObj 
     * @param InBounds 
     * @return 物体的inBounds完全在当前node以内返回true
     */
    bool Add(AActor* InObj, FBoxSphereBounds InBounds)
    {
        if (!BoxEncapsulates(Bounds, InBounds))
        {
            return  false;
        }
        SubAdd(InObj, InBounds);
        return true;
    }

    // OuterBounds的box部分 包住了 InnerBounds的box部分
    static bool BoxEncapsulates(const FBoxSphereBounds& OuterBounds, const FBoxSphereBounds& InnerBounds)
    {
        return  InnerBounds.GetBox().IsInside(OuterBounds.GetBox());
    }
private:
    // void Split
    
private:
    bool bHasChildren = false;
};



class TQxOctreeNode
{
public:
    // 划分node 的最小extent
    static constexpr float NodeMinExtent =  50.f;

    TQxOctreeNode(const TQxOctreeNode* InParent, const FVector& Origin, float InExtent)
        : Parent(InParent)
        , InclusiveNumElements(0)
        , bIsLeaf(true)
        , Extent(InExtent)
        , Bounds(Origin, FVector::OneVector * InExtent)
    {
        if (Extent >= NodeMinExtent)
        {
            SplitAndGetChildren();
            bIsLeaf = false;
        }
    }

    bool HasChildren()
    {
        return Children.Num() != 0;
    }

    void SplitAndGetChildren()
    {
        Children.Reset(8);

        // 更新设置child bounds
        const float ChildBaseExtent = Bounds.Extent.X / 2.f;
        static FVector BoxVertex[] =
            {
            FVector(-1, 1, -1),
            FVector(1, 1, -1),
            FVector(-1, 1, 1),
            FVector(1, 1, 1),
            FVector(-1, -1, -1),
            FVector(1, -1, -1),
            FVector(-1, -1, 1),
            FVector(1, -1, 1),
        };
        // Test
        static_assert(UE_ARRAY_COUNT(BoxVertex) == 8);
        Children.AddDefaulted(EIGHT);
        for (int32 i = 0; i < EIGHT; ++i)
        {
            // auto tmp = FBoxSphereBounds(
            //     Bounds.Center + BoxVertex[i] * ChildBaseExtent,
            //     FVector::OneVector * ChildActualExtent,
            //     ChildActualExtent * 2 // 这里先这样做
            //     );
            Children[i] = MakeShared<TQxOctreeNode>(
                this,
                Bounds.Center + BoxVertex[i] * ChildBaseExtent,
                ChildBaseExtent
                );
        }
    }

    template<typename LAMBDA>
    void TraverseNodeHierachy(LAMBDA&& InLambda)
    {
        InLambda(this);
        for (auto Child : Children)
        {
            InLambda(Child.Get());
        }
    }

    // 8个child
    TArray<TSharedPtr<TQxOctreeNode>, TInlineAllocator<8>> Children;

    // 内部的elements, 这里合理的做法是用模板参数，先这样做
    TArray<UObject*> Elements;

    // Parent
    const TQxOctreeNode* Parent;

    // 包括当前节点和子节点的element总数量
    uint32 InclusiveNumElements; 
    
    bool bIsLeaf;

    float Extent;
    
    // bounding 定义, 这里的extent 3个分量相等
    FBoxCenterAndExtent Bounds;
};

// template<typename  T>
class FTQxOctree
{
public:
    FTQxOctree(const FVector& InOrigin, float InExtent)
    {
        RootNode = MakeShared<TQxOctreeNode>(nullptr, InOrigin, InExtent);
    }
    
    // 当前octree 内物体的总数量
    int32 Count;

    // TSharedPtr<TQxOctreeNode<T>> RootNode;
    TSharedPtr<TQxOctreeNode> RootNode;

    // 
    float Looseness;

    // void DrawTree()
    // {
    //     RootNode->DrawNode();
    // }
};


USTRUCT(BlueprintType)
struct FQxOctree
{
    GENERATED_BODY()
public:
    FQxOctree()
        : Octree(FVector::ZeroVector, 100)
    {
        
    }
    FQxOctree(FVector Origin, float Extent)
        : Octree(Origin, Extent)
    {
        
    }

    FTQxOctree Octree;
};