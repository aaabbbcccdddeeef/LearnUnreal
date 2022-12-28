// Fill out your copyright notice in the Description page of Project Settings.


#include "QxTestViewportModify.h"

#include "Slate/SceneViewport.h"


// Sets default values
AQxTestViewportModify::AQxTestViewportModify()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AQxTestViewportModify::BeginPlay()
{
    Super::BeginPlay();

    UGameViewportClient* GameViewportClient = GetGameInstance()->GetGameViewportClient();
    FSceneViewport* SceneViewport = static_cast<FSceneViewport*>(GameViewportClient->Viewport);
    SceneViewport->ResizeFrame(600, 400, EWindowMode::Windowed);
}

// Called every frame
void AQxTestViewportModify::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

