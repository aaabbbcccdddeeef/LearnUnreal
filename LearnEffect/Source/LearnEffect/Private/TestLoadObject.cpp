// Fill out your copyright notice in the Description page of Project Settings.


#include "TestLoadObject.h"
#include "UObject/UObjectGlobals.h"
#include "Materials/MaterialInstance.h"
#include "Materials/Material.h"

// Sets default values
ATestLoadObject::ATestLoadObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATestLoadObject::BeginPlay()
{
	Super::BeginPlay();
	
	// TArray<int32, TInlineAllocator<int>> test;
}

// Called every frame
void ATestLoadObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UMaterial* ATestLoadObject::LoadTestMaterial()
{
	//LoadObject<UMaterial*>
	UMaterial* test2 = FindObject<UMaterial>(this, *AssetPath);
	UMaterial* test4 = FindObject<UMaterial>(GetWorld() , *AssetPath);
	UMaterial* test5 = FindObject<UMaterial>(nullptr, *AssetPath);
	UMaterial* test = LoadObject<UMaterial>(nullptr, *AssetPath);
	UMaterial* test6 = FindObject<UMaterial>(GetWorld(), *AssetPath);
	UMaterial* test3 = FindObject<UMaterial>(nullptr, *AssetPath);
	UMaterial* test7 = FindObject<UMaterial>(this, *AssetPath);
	//FindObjectFast()
	return test;
}

