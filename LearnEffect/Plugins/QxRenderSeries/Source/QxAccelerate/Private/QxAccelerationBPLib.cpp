// Fill out your copyright notice in the Description page of Project Settings.


#include "QxAccelerationBPLib.h"

#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

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

void UQxAccelerationBPLib::BuildTestOctree(FQxOctree& OutTree, const FVector& Origin, float Extent)
{
    OutTree = FQxOctree(Origin, Extent);
}

FQxOctree UQxAccelerationBPLib::BuiltTestOctree2(const FVector& Origin, float Extent, float InLooseness)
{
    return FQxOctree(Origin, Extent, InLooseness);
}

void UQxAccelerationBPLib::AddElement(FQxOctree& InOctree, UObject* InElement, const FBoxSphereBounds& InBounds)
{
    FBoxCenterAndExtent Bounds(InBounds.Origin, InBounds.BoxExtent);
    InOctree.AddElement(InElement, Bounds);
}


void UQxAccelerationBPLib::DrawQxOctree(const FQxOctree& InOctree,
                                        const UObject* WorldContextObject,
                                        FLinearColor InColor,
                                        float InLifeTime, float InThickness)
{
    InOctree.Octree.RootNode->TraverseNodeHierachy(
        [=](TQxOctreeNode* InNode)
        {
            UKismetSystemLibrary::DrawDebugBox(
                WorldContextObject,
                FVector(InNode->Bounds.Center),
                FVector(InNode->Extent * InNode->Looseness),
                InColor,
                FRotator::ZeroRotator,
                InLifeTime,
                InThickness);
        }
        );
}



