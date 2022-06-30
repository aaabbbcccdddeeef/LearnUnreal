// Fill out your copyright notice in the Description page of Project Settings.


#include "QxScriptedTexture.h"

#include "MovieSceneTracksComponentTypes.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Input/HittestGrid.h"
#include "Slate/WidgetRenderer.h"
#include "Widgets/SVirtualWindow.h"


// Sets default values for this component's properties


UQxScriptedTexture::UQxScriptedTexture(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UQxScriptedTexture::BeginPlay()
{
	Super::BeginPlay();

	if (FSlateApplication::IsInitialized())
	{
		SlateWindow = SNew(SVirtualWindow).Size(FVector2D(256.0, 256.0));
		SlateGrid = MakeShareable(new FHittestGrid());
	}
	check(SlateWindow.IsValid());
}

// Cleanup any Slate references when the component is being destroyed
void UQxScriptedTexture::OnUnregister()
{
	Super::OnUnregister();

	if (SlateGrid.IsValid())
	{
		SlateGrid.Reset();
	}
	if (SlateWindow.IsValid())
	{
		if (FSlateApplication::IsInitialized())
		{
			FSlateApplication::Get().UnregisterVirtualWindow(SlateWindow.ToSharedRef());
		}
		SlateWindow.Reset();
	}

	ScriptedTexture = nullptr;
	RenderingWidget = nullptr;
}


// Called every frame
void UQxScriptedTexture::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
}

// Render/Draw the Texture
void UQxScriptedTexture::Render(float DeltaTime)
{
	WidgetRenderer->DrawWindow(
		ScriptedTexture->GameThread_GetRenderTargetResource(),
		*SlateGrid.Get(),
		SlateWindow.ToSharedRef(),
		SlateGeometry,
		SlateGeometry.GetLayoutBoundingRect(),
		DeltaTime,
		false
		);
}

void UQxScriptedTexture::Resize(FIntPoint& NewSize)
{
	if (ScriptedTexture != nullptr)
	{
		ScriptedTexture->ResizeTarget(NewSize.X, NewSize.Y);
		// 重新创建slate window, 因为大小改变了
		UpdateSlateWindow();
	}
}

// Setup the Slate Window with the widget
void UQxScriptedTexture::UpdateSlateWindow()
{
	SlateWindow->SetContent(RenderingWidget->TakeWidget());
	SlateWindow->Resize(FVector2D(256, 256));
	SlateGeometry = FGeometry::MakeRoot(FVector2D(256, 256), FSlateLayoutTransform(1.f));
}

// 创建渲染需要的Render Target resource和user widget
void UQxScriptedTexture::Init()
{
	// Create widget to render into RTT
	// Load a class from a blueprint object.
	// Don't forget to add '_C' at the end to get the class
	FString Path = "WidgetBlueprint '/Game/UI/UMG_RenderMaterial.UMG_RenderMaterial_C'";
	TSubclassOf<UUserWidget> ClassWidget = LoadClass<UUserWidget>(nullptr, *Path);

	RenderingWidget = CreateWidget<UUserWidget>(GetWorld(), ClassWidget);

	// Create Render Target Resource
	FString Name = GetName() + "_ScriptTxt";
	ScriptedTexture = NewObject<UTextureRenderTarget2D>(this, UTextureRenderTarget2D::StaticClass(), *Name);
	check(ScriptedTexture);

	ScriptedTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
	ScriptedTexture->SizeX = 256.f;
	ScriptedTexture->SizeY = 256.f;
	ScriptedTexture->ClearColor = FLinearColor::Transparent;

	ScriptedTexture->UpdateResource();

	WidgetRenderer = new FWidgetRenderer(false, true);
	
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().RegisterVirtualWindow(SlateWindow.ToSharedRef());
	}

	UpdateSlateWindow();
}

AQxRenderWithSlate::AQxRenderWithSlate()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AQxRenderWithSlate::BeginPlay()
{
	Super::BeginPlay();

	ScriptedTexture = NewObject<UQxScriptedTexture>(this, UQxScriptedTexture::StaticClass());
	ScriptedTexture->AttachToComponent(
		GetRootComponent(),
		FAttachmentTransformRules::SnapToTargetIncludingScale
		);
	ScriptedTexture->RegisterComponent();
	ScriptedTexture->Init();
}

void AQxRenderWithSlate::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ScriptedTexture)
	{
		ScriptedTexture->Render(DeltaSeconds);
	}
}


