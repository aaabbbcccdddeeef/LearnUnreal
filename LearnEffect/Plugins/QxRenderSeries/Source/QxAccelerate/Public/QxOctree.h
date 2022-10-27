#pragma once
#include "CoreMinimal.h"
#include "QxAccelerate.h"
#include "UObject/Object.h"
#include "QxOctree.generated.h"

// 这个Octreee的实现大部分参考 github 上的Unity Octree的实现，UE4自带的TOctree2还没明白

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
    // 每个节点允许添加的最大数量的objects，inclusive，包括16个
    static constexpr int32 MaxObjNum = 8;

    TQxOctreeNode(const TQxOctreeNode* InParent,
        const FVector& Origin,
        float InExtent,
        float InLooseness = 1.0f)
        : Parent(InParent)
        , InclusiveNumElements(0)
        , bIsLeaf(true)
        , Extent(InExtent)
        , Bounds(Origin, FVector::OneVector * InExtent)
    {
        Looseness = FMath::Clamp(InLooseness, 1.0f, 2.0f);
        // if (Extent > NodeMinExtent)
        // {
        //     SplitAndGetChildren();
        //     bIsLeaf = false;
        // }
        // else
        // {
        //     int32 test = 5;
        // }
    }

    bool HasChildren()
    {
        return Children.Num() != 0;
    }

    void SplitAndGenearateChildren()
    {
        Children.Reset(8);

        // 更新设置child bounds
        const float ChildBaseExtent = Extent / 2.f;
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
                ChildBaseExtent,
                Looseness
                );
        }
    }

    FBoxCenterAndExtent GetActualBounds() const
    {
        FBoxCenterAndExtent ActulBounds(
            Bounds.Center,
            Bounds.Extent * Looseness
            );
        return ActulBounds;
    }
    
    template<typename LAMBDA>
    void TraverseNodeHierachy(LAMBDA&& InLambda)
    {
        InLambda(this);
        for (auto Child : Children)
        {
            // InLambda(Child.Get());
            Child->TraverseNodeHierachy(Forward<LAMBDA>(InLambda));   
        }
    }

    bool HasAnyObjects();

    int32 FindBestFitChild(const FVector4& ObjCenter)
    {
        // return (ObjCenter.X <= Bounds.Center.X ? 0 : 2) +
        //     (ObjCenter.Y <= Bounds.Center.Y ? 0 : 1) +
        //         (ObjCenter.Z <= Bounds.Center.Z ? 0 : 4);

        FVector Center = Bounds.Center;
        return (ObjCenter.X <= Center.X ? 0 : 1) + (ObjCenter.Y >= Center.Y ? 0 : 4) + (ObjCenter.Z <= Center.Z ? 0 : 2);
    }


    /**
     * @brief 
     * @param InElement 
     * @param InBounds 
     * @return 输入物体的bounds在当前节点内才添加
     */
    bool Add(UObject* InElement, const FBoxCenterAndExtent& InBounds);

    void GetWithInFrustum(const FConvexVolume& InFrustum, TArray<UObject*>& OutElements);

    bool IsColliding(const FBoxCenterAndExtent& CheckBounds);

    void GetCollidings(const FBoxCenterAndExtent& CheckBounds, TArray<UObject*>& Result);

    void GetCollidings(const FVector& Start, const FVector& End,  TArray<UObject*>& OutResult);


    // 8个child，这里或许有更好的数据结构选择
    // 这个8个child按照其中心和当前node的中心的关系，encode成1维的index, 算法参照FindBestFitChild
    TArray<TSharedPtr<TQxOctreeNode>, TInlineAllocator<8>> Children;

    struct OctreeObject
    {
        OctreeObject()
            : OctreeObject(nullptr, FBoxCenterAndExtent(FVector::ZeroVector, FVector::OneVector))
        {  }
        OctreeObject(UObject* InObject, const FBoxCenterAndExtent& InBounds)
            : Object(InObject), ObjBounds(InBounds)
        {
            
        }
        UObject* Object;
        FBoxCenterAndExtent ObjBounds;
    };
    
    // 内部的elements, 这里合理的做法是用模板参数，先这样做
    TArray<OctreeObject> Elements;

    // Parent
    const TQxOctreeNode* Parent;

    // 包括当前节点和子节点的element总数量
    uint32 InclusiveNumElements; 
    
    bool bIsLeaf;

    float Extent;
    
    // bounding 定义, 这里的extent 3个分量相等
    FBoxCenterAndExtent Bounds;

    // 当前节点的松散度，默认的非松散的八叉树是1.0， 非松散的大于1.0小于2.0
    float Looseness;

    void SetSelected(bool InIsSelected)
    {
        bIsSelected = InIsSelected;
    }

    bool IsSelected()
    {
        return bIsSelected;
    }
private:
    bool bIsSelected = false;
    // 当前节点是不是最深的，
    bool IsSplitable()
    {
        return  Extent > NodeMinExtent;
    }
    
    // 这里描述的节点的分裂策略实现,
    // 当前节点的element数量小于节点允许的最大数量，直接添加新元素，不分裂
    // 如果当前节点不可分裂，也直接添加
    // 超过允许的最大数量时，分裂8个子节点
    void SubAdd(UObject* InElement, const FBoxCenterAndExtent& InBounds)
    {
        if (!IsSplitable() || Elements.Num() < MaxObjNum)
        {
            Elements.Add(OctreeObject{InElement, InBounds});
            return;
        }

        if (Children.Num() <= 0)
        {
            SplitAndGenearateChildren();
            if (Children.Num() <= 0)
            {
                UE_LOG(QxAccelerate, Warning, TEXT("Children create failed for unkown reason. early exit"));
                return;
            }

            DistributeObjectsToChildren();
        }


        int32 BestFitIndex = FindBestFitChild(InBounds.Center);
        if (Encapsulates(Children[BestFitIndex]->Bounds, InBounds))
        {
            Children[BestFitIndex]->SubAdd(InElement, InBounds);
        }
        else
        {
            Elements.Add(OctreeObject(InElement, InBounds));
        }
    }

    inline  bool Encapsulates(const FBoxCenterAndExtent& Outer, const FBoxCenterAndExtent& Inner)
    {
        // return Inner.GetBox().IsInside(Outer.GetBox());
        return Outer.GetBox().IsInside(Inner.GetBox());
    }

    void DistributeObjectsToChildren()
    {
        check(Children.Num() > 0);
        for (int32 i = Elements.Num() - 1; i >= 0; --i)
        {
            OctreeObject& CurElement = Elements[i];
            int32 BestFitChild = FindBestFitChild(CurElement.ObjBounds.Center);
            if (Encapsulates(Children[BestFitChild]->Bounds, CurElement.ObjBounds))
            {
                Children[BestFitChild]->SubAdd(CurElement.Object, CurElement.ObjBounds);
                Elements.RemoveAtSwap(i);
            }
        }
    }
};

// template<typename  T>
// #TODO 把element 类型换成模板类型
class TQxOctree
{
public:
    TQxOctree()
        : TQxOctree(FVector::ZeroVector, 10, 1.f)
    {
    }
    
    TQxOctree(const FVector& InOrigin, float InExtent, float InLooseness = 1.f)
    {
        RootNode = MakeShared<TQxOctreeNode>(nullptr, InOrigin, InExtent, InLooseness);
    }

    TQxOctree(const TQxOctree& OtherOctree)
    {
        RootNode.Reset();
        RootNode = MakeShared<TQxOctreeNode>(nullptr,
            OtherOctree.RootNode->Bounds.Center,
            OtherOctree.RootNode->Extent,
            OtherOctree.Looseness);
    }

    void AddElement(UObject* InElement, const FBoxCenterAndExtent& Bounds);

    // 使当前八叉树沿growDirection增长到2倍
    void Grow(FVector GrowDirection)
    {
        int32 xDirection = GrowDirection.X >= 0 ? 1 : -1;
        int32 yDirection = GrowDirection.Y >= 0 ? 1 : -1;
        int32 zDirection = GrowDirection.Z >= 0 ? 1 : -1;
        auto OldRoot = RootNode;
        const float OldExtent = OldRoot->Extent;
        const float NewExtent = OldRoot->Extent * 2;
        FVector NewCenter = OldRoot->Bounds.Center +
            FVector(xDirection * OldExtent, yDirection * OldExtent, zDirection * OldExtent);

        RootNode = MakeShared<TQxOctreeNode>(nullptr, NewCenter, NewExtent, Looseness);

        // 将原来的RootNode作为新的octree的一部分，其他用新节点
        if (OldRoot->HasAnyObjects())
        {
            int32 OldRootIndexInNew = RootNode->FindBestFitChild(OldRoot->Bounds.Center);
            RootNode->Children.AddUninitialized(8);
            for (int i = 0; i < EIGHT; ++i)
            {
                if (i == OldRootIndexInNew)
                {
                    RootNode->Children[i] = OldRoot;
                }
                else
                {
                    xDirection = i % 2 == 0 ? -1 : 1;
                    yDirection = i > 3 ? -1 : 1;
                    zDirection = (i < 2 || (i > 3 && i < 6)) ? -1 : 1;
                    FVector NewChildCenter = NewCenter +
                        FVector(xDirection * OldExtent, yDirection * OldExtent, zDirection * OldExtent);
                    RootNode->Children[i] = MakeShared<TQxOctreeNode>(RootNode.Get(), NewChildCenter, RootNode->Extent / 2, Looseness);
                }
            }
        }
    }


    TArray<UObject*> GetWithinFrustum(const FConvexVolume& InFrustum)
    {
        TArray<UObject*> Result;
        RootNode->GetWithInFrustum(InFrustum, Result);
        return MoveTemp(Result);
    }

    bool IsColliding(const FBoxCenterAndExtent& CheckBounds)
    {
        return RootNode->IsColliding(CheckBounds);
    }

    void GetCollidings(const FBoxCenterAndExtent& CheckBounds, TArray<UObject*>& OutResult)
    {
        RootNode->GetCollidings(CheckBounds, OutResult);
    }

    void GetCollidings(const FVector& Start,const FVector& End, TArray<UObject*>& OutResult)
    {
        RootNode->GetCollidings(Start, End, OutResult);
    }

    void ClearSelectedState()
    {
        RootNode->TraverseNodeHierachy(
            [](TQxOctreeNode* InNode)
            {
                InNode->SetSelected(false);
            }
            );
    }

    // 当前octree 内物体的总数量
    int32 ElementCount;

    // TSharedPtr<TQxOctreeNode<T>> RootNode;
    TSharedPtr<TQxOctreeNode> RootNode;

    // 
    float Looseness;

    // void DrawTree()
    // {
    //     RootNode->DrawNode();
    // }
};

UCLASS(BlueprintType)
class UQxOctree : public UObject
{
    GENERATED_BODY()
public:
    // UQxOctree(FVector Origin, float Extent, float InLooseness = 1.f)
    //     : Octree(Origin, Extent, InLooseness)
    // {
    //     
    // }
    // UQxOctree()
    //     : UQxOctree(FVector::ZeroVector, 100)
    // {
    //     
    // }

    void Init(FVector Origin, float Extent, float InLooseness = 1.f)
    {
        Octree = TQxOctree(Origin, Extent, InLooseness);
        bIsInitialized = true;
    }
    
    //
    // UFUNCTION(BlueprintCallable, Category="QxOctree")
    // TArray<UObject*> GetWithinFrustum(const FConvexVolume& InFrustum)
    // {
    //     return  Octree.GetWithinFrustum(InFrustum);
    // }

    UFUNCTION(BlueprintCallable, Category="QxOctree")
    TArray<UObject*> GetWithinFrustum(const TArray<FPlane>& InFrustumPlanes)
    {
        FConvexVolume InFrustum =  FConvexVolume(TArray<FPlane, TInlineAllocator<6>>(InFrustumPlanes));
        return  Octree.GetWithinFrustum(InFrustum);
    }

    UFUNCTION(BlueprintCallable, Category="QxOctree")
    bool IsColliding(const FBoxSphereBounds& CheckBounds)
    {
        return Octree.IsColliding(FBoxCenterAndExtent(CheckBounds.Origin, CheckBounds.BoxExtent));
    }

    UFUNCTION(BlueprintCallable, Category="QxOctree")
    void GetCollidings(const FBoxSphereBounds& CheckBounds, TArray<UObject*>& OutResult)
    {
        Octree.GetCollidings(FBoxCenterAndExtent(CheckBounds.Origin, CheckBounds.BoxExtent), OutResult);
    }

    UFUNCTION(BlueprintCallable, Category="QxOctree")
    void GetVectorCollidings(const FVector& Start,const FVector& End, TArray<UObject*>& OutResult)
    {
        Octree.GetCollidings(Start, End, OutResult);
    }

    UFUNCTION(BlueprintCallable, Category="QxOctree")
    void AddElement(UObject* InElement, const FBoxSphereBounds& InBounds)
    {
        check(bIsInitialized);
        Octree.AddElement(InElement, FBoxCenterAndExtent(InBounds.Origin, InBounds.BoxExtent));
    }

    UFUNCTION(BlueprintPure, Category="QxOctree")
    FVector GetCenter() const
    {
        return Octree.RootNode->Bounds.Center;
    }

    // 这里假设我们用x/y/z相等的extent
    UFUNCTION(BlueprintPure, Category="QxOctree")
    float GetExtent() const
    {
        return Octree.RootNode->Extent;
    }

    UFUNCTION(BlueprintCallable, Category="QxOctree")
    void ClearSelected()
    {
        Octree.ClearSelectedState();
    }

#pragma region Test
    
    
#pragma endregion

    TQxOctree Octree;
private:
    bool bIsInitialized = false;
};