// Fill out your copyright notice in the Description page of Project Settings.


#include "QxAccelerationBPLib.h"

#include <stdexcept>

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
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

UQxOctree* UQxAccelerationBPLib::BuildTestOctree(const FVector& Origin, float Extent)
{
    // OutTree = UQxOctree(Origin, Extent);
    UQxOctree* testOctree = NewObject<UQxOctree>();
    testOctree->Init(Origin, Extent);
    return testOctree;
}

UQxOctree* UQxAccelerationBPLib::BuiltTestOctree2(const FVector& Origin, float Extent, float InLooseness)
{
    UQxOctree* testOctree = NewObject<UQxOctree>();
    testOctree->Init(Origin, Extent, InLooseness);
    return testOctree;
}

void UQxAccelerationBPLib::AddOctreeElement(UQxOctree* InOctree, UObject* InElement, const FBoxSphereBounds& InBounds)
{
    // FBoxCenterAndExtent Bounds(InBounds.Origin, InBounds.BoxExtent);
    // InOctree.AddElement(InElement, Bounds);
    InOctree->AddElement(InElement, InBounds);
}



void UQxAccelerationBPLib::DrawQxOctree(const UQxOctree* InOctree,
                                        const UObject* WorldContextObject,
                                        FLinearColor InColor,
                                        float InLifeTime, float InThickness)
{
    InOctree->Octree.RootNode->TraverseNodeHierachy(
        [=](TQxOctreeNode* InNode)
        {
            FLinearColor DebugColor = InNode->IsSelected() ? FLinearColor::Red : InColor;
            UKismetSystemLibrary::DrawDebugBox(
                WorldContextObject,
                FVector(InNode->Bounds.Center),
                FVector(InNode->Extent * InNode->Looseness),
                DebugColor,
                FRotator::ZeroRotator,
                InLifeTime,
                InThickness);
        }
        );
}

void UQxAccelerationBPLib::GetCameraFrustumPlanes(const UObject* WorldContextObject, TArray<FPlane>& OutPlanes)
{
     APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(WorldContextObject, 0);

    FVector CameraLocation = CameraManager->GetCameraLocation();
    FRotator CameraRotation = CameraManager->GetCameraRotation();
    FVector CameraForward = UKismetMathLibrary::GetForwardVector(CameraRotation);
    FVector CameraRight = UKismetMathLibrary::GetRightVector(CameraRotation);
    FVector CameraUp = UKismetMathLibrary::GetUpVector(CameraRotation);

    FVector NearClipCenter = CameraLocation +  GEngine->NearClipPlane * CameraForward;
    FPlane NearClipPlane(NearClipCenter, CameraForward);
    // TODO 关于如何获得far clipping plane有些疑惑
    FPlane FarClipPlane(CameraLocation + CameraForward * 1000000, CameraForward);

    float HozLength = 0;
    float VerLength = 0;
    const float HozHalfAngleInRedians = FMath::DegreesToRadians(CameraManager->GetFOVAngle() * 0.5f);
    APlayerController* PC =  UGameplayStatics::GetPlayerController(WorldContextObject, 0);
    int32 ViewX,ViewY;
    PC->GetViewportSize(ViewX, ViewY);
    check(ViewX != 0 && ViewY != 0);
    float CameraAspect = (float)ViewX/ViewY;
    if (!CameraManager->IsOrthographic())
    {
        HozLength = GEngine->NearClipPlane * FMath::Tan(HozHalfAngleInRedians);
        VerLength = HozLength / CameraAspect;
    }
    else
    {
        // #TODO 正交投影
        check(false);
    }
    
    // 近裁剪面的4个点
    FVector LeftUp = NearClipCenter - CameraRight * HozLength + CameraUp * VerLength;
    FVector LeftBottom = NearClipCenter - CameraRight * HozLength - CameraUp * VerLength;
    FVector RightUp = NearClipCenter + CameraRight * HozLength + CameraUp * VerLength;
    FVector RightBottom = NearClipCenter + CameraRight * HozLength - CameraUp * VerLength;

    // 注意： 这里传入的3个点的顺序问题
    FPlane LeftPlane(CameraLocation, LeftUp, LeftBottom);
    FPlane RightPlane(CameraLocation, RightBottom, RightUp);
    FPlane UpPlane(CameraLocation, RightUp, LeftUp);
    FPlane BottomPlane(CameraLocation, LeftBottom, RightBottom);

    OutPlanes.Add(NearClipPlane);
    OutPlanes.Add(FarClipPlane);
    OutPlanes.Add(LeftPlane);
    OutPlanes.Add(UpPlane);
    OutPlanes.Add(RightPlane);
    OutPlanes.Add(BottomPlane);
}

TArray<UObject*> UQxAccelerationBPLib::TestCameraFrustumIntersect(
    const UObject* WorldContextObject,
    UQxOctree* InOctree)
{
    APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(WorldContextObject, 0);


    FVector CameraLocation = CameraManager->GetCameraLocation();
    FRotator CameraRotation = CameraManager->GetCameraRotation();
    FVector CameraForward = UKismetMathLibrary::GetForwardVector(CameraRotation);
    FVector CameraRight = UKismetMathLibrary::GetRightVector(CameraRotation);
    FVector CameraUp = UKismetMathLibrary::GetUpVector(CameraRotation);

    FVector NearClipCenter = CameraLocation +  GEngine->NearClipPlane * CameraForward;
    FPlane NearClipPlane(NearClipCenter, CameraForward);
    // TODO 关于如何获得far clipping plane有些疑惑
    FPlane FarClipPlane(CameraLocation + CameraForward * 1000000, CameraForward);

    float HozLength = 0;
    float VerLength = 0;
    const float HozHalfAngleInRedians = FMath::DegreesToRadians(CameraManager->GetFOVAngle() * 0.5f);
    APlayerController* PC =  UGameplayStatics::GetPlayerController(WorldContextObject, 0);
    int32 ViewX,ViewY;
    PC->GetViewportSize(ViewX, ViewY);
    check(ViewX != 0 && ViewY != 0);
    float CameraAspect = (float)ViewX/ViewY;
    if (!CameraManager->IsOrthographic())
    {
        HozLength = GEngine->NearClipPlane * FMath::Tan(HozHalfAngleInRedians);
        VerLength = HozLength / CameraAspect;
    }
    else
    {
        // #TODO 正交投影
        check(false);
    }
    
    // 近裁剪面的4个点
    FVector LeftUp = NearClipCenter - CameraRight * HozLength + CameraUp * VerLength;
    FVector LeftBottom = NearClipCenter - CameraRight * HozLength - CameraUp * VerLength;
    FVector RightUp = NearClipCenter + CameraRight * HozLength + CameraUp * VerLength;
    FVector RightBottom = NearClipCenter + CameraRight * HozLength - CameraUp * VerLength;

    // 注意： 这里传入的3个点的顺序问题
    FPlane LeftPlane(CameraLocation, LeftUp, LeftBottom);
    FPlane RightPlane(CameraLocation, RightBottom, RightUp);
    FPlane UpPlane(CameraLocation, RightUp, LeftUp);
    FPlane BottomPlane(CameraLocation, LeftBottom, RightBottom);

    FConvexVolume TestFrustum;
    TestFrustum.Planes.Add(NearClipPlane);
    TestFrustum.Planes.Add(FarClipPlane);
    TestFrustum.Planes.Add(LeftPlane);
    TestFrustum.Planes.Add(UpPlane);
    TestFrustum.Planes.Add(RightPlane);
    TestFrustum.Planes.Add(BottomPlane);
    TestFrustum.Init();

    return InOctree->Octree.GetWithinFrustum(TestFrustum);
}

TArray<UObject*> UQxAccelerationBPLib::TestCameraFrustumIntersect2(const UObject* WorldContextObject,
    UQxOctree* InOctree, const TArray<FPlane>& InFrustumPlanes)
{
    return InOctree->GetWithinFrustum(InFrustumPlanes);
}


float UQxAccelerationBPLib::GetNearClipPlane()
{
    return  GEngine->NearClipPlane;
}

FMinimalViewInfo UQxAccelerationBPLib::GetCurViewInfo(APlayerCameraManager* InCameraManager)
{
    if (InCameraManager)
    {
        return  InCameraManager->GetCameraCachePOV();
    }
    else
    {
        return FMinimalViewInfo();
    }
}






