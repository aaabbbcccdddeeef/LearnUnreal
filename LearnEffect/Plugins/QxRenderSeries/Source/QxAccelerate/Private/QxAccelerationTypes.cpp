#include "QxAccelerationTypes.h"

#include "QxAccelerate.h"

void FQxKdtree::BuildKdtree(const TArray<FVector>& InPoints)
{
    Clear();
    Points = InPoints;

    TArray<int32> Indices;
    Indices.AddUninitialized(InPoints.Num());
    for (int32 i = 0; i < InPoints.Num(); ++i)
    {
        Indices[i] = i;
    }

    TArrayView<int32> IndicesSpan(Indices);
    Root = BuildNode(*this, IndicesSpan, 0);
}

void FQxKdtree::SearchKNNPoint(const FVector& TargetPoint, int32 k, TArray<FQxKdtreeNode*>& OutNodes) 
{
    OutNodes.Empty();
    // SearchKNNPoint_Recursive(TargetPoint, k, )
}

TSharedPtr<FQxKdtreeNode> FQxKdtree::BuildNode(FQxKdtree& Tree, TArrayView<int32> IndiceSpan, int32 Depth)
{
    if (IndiceSpan.Num() <= 0)
    {
        return nullptr;
    }

    const int32 Axis = Depth % 3;
    const int32 Middle = (IndiceSpan.Num() - 1) / 2;

    // #TODO 这里其实作用只是相当于 std::nth_element 没必要用完整的sort
    // 替换成std::nth_element
    FQxKdtree* TreePtr = &Tree;
    Algo::Sort(IndiceSpan, [TreePtr, Axis](int Lhs, int Rhs)
       {
           if (TreePtr->Points[Lhs][Axis] < TreePtr->Points[Rhs][Axis])
           {
               return true;
           } else
           {
               return  false;
           }
           // return 0;
       });

    

    // TUniquePtr<FQxKdtreeNode> NewNode = MakeUnique<FQxKdtreeNode>();
    TSharedPtr<FQxKdtreeNode> NewNode = MakeShared<FQxKdtreeNode>();
    NewNode->Index = IndiceSpan[Middle];
    NewNode->Axis = Axis;
    TArrayView<int32> LeftSpan(IndiceSpan.GetData(), Middle);
    TArrayView<int32> RightSpan(IndiceSpan.GetData() + Middle + 1, IndiceSpan.Num() - Middle - 1);
    
    NewNode->ChildLeft = BuildNode(Tree, LeftSpan, Depth + 1);
    NewNode->ChildRight = BuildNode(Tree, RightSpan, Depth + 1);
    
    // return MoveTemp(NewNode);
    return  NewNode;
}

void FQxKdtree::CollectFromKdtree(const FVector& Center, float Radius, TArray<int32>& OutIndices) const
{
    CollectFromKdtree(Root.Get(), Center, Radius, OutIndices);
}

void FQxKdtree::DumpKdtree()
{
    UE_LOG(QxAccelerate, Warning, TEXT("========== DUMP FKdtree =========="));
    DumpKdtree(Root.Get());
    UE_LOG(LogTemp, Display, TEXT("=================================="));
}

void FQxKdtree::DumpKdtree(FQxKdtreeNode* InNode)
{
    if (!InNode)
    {
        return;
    }
    DumpNode(InNode);
    if (InNode->ChildLeft.IsValid())
    {
        DumpNode(InNode->ChildLeft.Get());
    }
    if (InNode->ChildLeft.IsValid())
    {
        DumpNode(InNode->ChildRight.Get());
    }
}

void FQxKdtree::DumpNode(FQxKdtreeNode* Node)
{
    if (Node == nullptr)
    {
        return;
    }

    FString Left = "null";
    FString Right = "null";

    if (Node->ChildLeft)
    {
        Left = FString::FromInt(Node->ChildLeft->Index);
    }
    if (Node->ChildRight)
    {
        Right = FString::FromInt(Node->ChildRight->Index);
    }

    UE_LOG(LogTemp, Display, TEXT("[%d] value=(%f, %f, %f), axis=%d, child_left=%s, child_right=%s"),
        Node->Index,
        Points[Node->Index][0],
        Points[Node->Index][1],
        Points[Node->Index][2],
        Node->Axis,
        *Left,
        *Right);
}

void FQxKdtree::CollectFromKdtree(const FQxKdtreeNode* InNode, const FVector& Vector, float Radius,
                                  TArray<int32>& OutIndices) const
{
    if (!InNode)
    {
        return;
    }
    
}

FQxKdtreeNode* FQxKdtree::SearchNearestNode(const FVector& InPoint) const
{
    FQxKdtreeNode* ResultNode = nullptr;
    double MinDist =  TNumericLimits<double>::Max();
    SearchNearestNode(InPoint, Root.Get(), ResultNode, MinDist);
    return ResultNode;
}

void FQxKdtree::SearchNearestNode(const FVector& TargetPoint, FQxKdtreeNode* InNode, FQxKdtreeNode*& ResultNode,
    double& MinDist) const
{
    if (InNode == nullptr)
    {
        return;    
    }
    const FVector& CurrentPoint = Points[InNode->Index];
    const double TargetPToCurrentP = (TargetPoint - CurrentPoint).Size();
    if (TargetPToCurrentP < MinDist)
    {
        MinDist = TargetPToCurrentP;
        ResultNode = InNode;
    }

    const int32 axis = InNode->Axis;
    bool bUseLeftBranch = TargetPoint[axis] < CurrentPoint[axis];
    SearchNearestNode(TargetPoint,
        bUseLeftBranch ? InNode->ChildLeft.Get() : InNode->ChildRight.Get(),
        ResultNode,
        MinDist);
    
    const double TargetToSplitPlane = FMath::Abs(TargetPoint[axis] - CurrentPoint[axis]);
    if (TargetToSplitPlane < MinDist)
    {
        SearchNearestNode(TargetPoint,
            !bUseLeftBranch ? InNode->ChildLeft.Get() : InNode->ChildRight.Get(),
            ResultNode,
            MinDist);
    }
}


