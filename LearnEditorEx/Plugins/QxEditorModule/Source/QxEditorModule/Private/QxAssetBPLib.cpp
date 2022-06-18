// Fill out your copyright notice in the Description page of Project Settings.


#include "QxAssetBPLib.h"

void UQxAssetBPLib::Test1()
{
	UE_LOG(LogTemp, Warning, TEXT("Qx Test is invokded"));
}

bool UQxAssetBPLib::SetMeshLODWithAnotherMesh(UStaticMesh* InTargetMesh, UStaticMesh* InSrcMesh, int32 InLODIndex)
{
	return  false;
}
