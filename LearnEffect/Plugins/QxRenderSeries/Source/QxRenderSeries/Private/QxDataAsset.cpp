// Fill out your copyright notice in the Description page of Project Settings.


#include "QxDataAsset.h"

#include "Kismet/KismetSystemLibrary.h"

UQxDataAsset::UQxDataAsset(const FObjectInitializer& InObjectInitializer): Super(InObjectInitializer)
{
	UE_LOG(LogTemp, Warning, TEXT("UQxDataAsset constructor is invoked"));
}

void UQxDataAsset::PostInitProperties()
{
	Super::PostInitProperties();
}

void UQxDataAsset::PostLoad()
{
	Super::PostLoad();
}

FString UQxDataAsset::PrintObjectInfo()
{
	FString OutputStr = TEXT("Object Path = ") + UKismetSystemLibrary::GetPathName(this) + TEXT("\n");
	return  FString("");
}
