// Fill out your copyright notice in the Description page of Project Settings.


#include "QxRenderUtils.h"

#include "Kismet/KismetSystemLibrary.h"

TArray<FVector> FQxRenderUtils::GetVertexPositonsWS(UStaticMeshComponent* InMeshComponent)
{
	TArray<FVector> meshVerticesWS;
	if (!InMeshComponent)
	{
		return meshVerticesWS;
	}

	UStaticMesh* staticMesh = InMeshComponent->GetStaticMesh();
	if (!staticMesh)
	{
		return  meshVerticesWS;
	}

	// 检查mesh 是否有lod
	if (false == (staticMesh->GetRenderData()->LODResources.Num() > 0))
	{
		return meshVerticesWS;
	}

	// Get the vertices
	FPositionVertexBuffer* vertexBuffer =  &(staticMesh->GetRenderData()->LODResources[0].VertexBuffers.PositionVertexBuffer);
	for (uint32 i = 0; i < vertexBuffer->GetNumVertices(); ++i)
	{
		FVector vertexLS = vertexBuffer->VertexPosition(i);

		FVector vertexWS = InMeshComponent->GetComponentTransform().TransformPosition(vertexLS);

		meshVerticesWS.Add(vertexWS);
	}
	return  meshVerticesWS;
}

bool FQxRenderUtils::RayCastHit(const FVector& RayOrigin, const FVector& RayDirection, float RayMarchingLength,
	FHitResult& OutHitResult, AActor* InActor)
{
	const TArray<AActor*> IgActor;

	FVector StartPos = RayOrigin;
	FVector EndPos = RayOrigin + RayDirection * RayMarchingLength;

	return UKismetSystemLibrary::LineTraceSingle(
			InActor,
			StartPos,
			EndPos,
			ETraceTypeQuery::TraceTypeQuery1,
			false,
			IgActor,
			EDrawDebugTrace::Type::None,
			OutHitResult,
			true,
			FLinearColor::Blue,
			FLinearColor::Red,
			1.0f
		);
}
