// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QxCustomObject.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class QXASSETS_API UQxCustomObject : public UObject
{
	GENERATED_BODY()
	
};

USTRUCT(BlueprintType)
struct FColorStruct
{
	GENERATED_USTRUCT_BODY()
public:
	//字符串，就是上面要显示的内容，这里注意，UPROPERTY宏要加上EditAnywhere，这样PropertyHandle才能获取到反射数据
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Color;
	//用于记录Checkbox的状态
	UPROPERTY(EditAnywhere)
	bool bRed;

	FColorStruct() : Color("Red"), bRed(true){}
};
