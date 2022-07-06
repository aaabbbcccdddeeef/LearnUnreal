// Fill out your copyright notice in the Description page of Project Settings.


#include "QxUserWidget.h"

#include "Components/TextBlock.h"

#define LOCTEXT_NAMESPACE "QxUI"

void UQxUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TitleLabe)
	{
		TitleLabe->SetText(FText::FromString(TEXT("Hello world")));
	}
}



const FText UQxOverlay::GetPaletteCategory()
{
	// return Super::GetPaletteCategory();

	return LOCTEXT("QxOverlay", "QxTestOverly");
}

TSharedRef<SWidget> UQxOverlay::RebuildWidget()
{
	return Super::RebuildWidget();
}


TSharedRef<SWidget> UQxOverlay::RebuildDesignWidget(TSharedRef<SWidget> Content)
{
	auto res = Super::RebuildDesignWidget(Content);

	for (UPanelSlot* InSlot : Slots)
	{
		UE_LOG(LogTemp, Warning, TEXT("slots name = %s"), *InSlot->GetName())
	}
	
	return  res;
}

#undef LOCTEXT_NAMESPACE