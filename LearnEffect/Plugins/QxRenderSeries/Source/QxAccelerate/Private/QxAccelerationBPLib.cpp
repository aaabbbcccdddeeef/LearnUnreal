// Fill out your copyright notice in the Description page of Project Settings.


#include "QxAccelerationBPLib.h"

void UQxAccelerationBPLib::BuildKdtree(FQxKdtree& Tree, const TArray<FVector> InPoints)
{
    // ClearKdtree(Tree);
    Tree.BuildKdtree(InPoints);
}

void UQxAccelerationBPLib::ClearKdtree(FQxKdtree& Tree)
{
    Tree.Clear();
}

void UQxAccelerationBPLib::ValidateKdtree(const FQxKdtree& Tree)
{
}

void UQxAccelerationBPLib::DumpKdtreeToConsole(FQxKdtree& Tree)
{
    Tree.DumpKdtree();
}

void UQxAccelerationBPLib::CollectFromKdtree(const FQxKdtree& Tree, const FVector Center, float Radius,
                                             TArray<int32>& OutIndices, TArray<FVector>& OutPoints)
{
    Tree.CollectFromKdtree(Center, Radius, OutIndices);
    OutPoints.Empty(OutIndices.Num());
    for (int32 Index : OutIndices)
    {
        OutPoints.Add(Tree.Points[Index]);
    }
}

FVector UQxAccelerationBPLib::SearchNerestPoint(const FQxKdtree& Tree, const FVector& TargetPoint)
{
    return Tree.SearchNearestPoint(TargetPoint);
}

void UQxAccelerationBPLib::SearchKNNPoint(FQxKdtree& Tree, const FVector& TargetPoint, int32 k,
    TArray<FVector>& OutPoints)
{
    OutPoints.Empty();

    // 
    TArray<FQxKdtreeNode*> OutNodes;
    Tree.SearchKNNPoint(TargetPoint, k, OutNodes);
    for (FQxKdtreeNode* Node : OutNodes)
    {
        OutPoints.Add(Tree.Points[Node->Index]);
    }
}
