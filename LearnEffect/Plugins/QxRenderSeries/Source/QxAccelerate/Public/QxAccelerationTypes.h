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
