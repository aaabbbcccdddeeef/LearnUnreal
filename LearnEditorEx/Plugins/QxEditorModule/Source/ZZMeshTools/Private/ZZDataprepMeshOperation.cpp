// Fill out your copyright notice in the Description page of Project Settings.


#include "ZZDataprepMeshOperation.h"

void UDataprepCreateProxyMeshEachOperation::OnExecution_Implementation(const FDataprepContext& InContext)
{
	using FMeshArrayPtr = TArray<UStaticMeshComponent*>;
	// 每个actor 的mesh component作为一个FMeshArrayPtr
	TArray<FMeshArrayPtr> actorMeshComs;
	UWorld* CurrentWorld = nullptr;
}
