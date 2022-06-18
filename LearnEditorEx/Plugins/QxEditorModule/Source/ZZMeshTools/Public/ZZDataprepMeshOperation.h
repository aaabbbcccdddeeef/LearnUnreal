// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataprepOperation.h"
#include "ZZDataprepMeshOperation.generated.h"

/**
 * 
 */
UCLASS(Category = ObjectOperation, Meta = (DisplayName = "Create Proxy Mesh Each", ToolTip = "create mesh proxy for every mesh indivivisually"))
class ZZMESHTOOLS_API UDataprepCreateProxyMeshEachOperation : public UDataprepEditingOperation
{
	GENERATED_BODY()
public:
	virtual FText GetCategory_Implementation() const override
	{
		return FDataprepOperationCategories::ActorOperation;
	};
protected:
	virtual void OnExecution_Implementation(const FDataprepContext& InContext) override;
private:
	
public:
	UPROPERTY(EditAnywhere, Category = ProxySettings,  meta = (UIMin = 0, UIMax = 100))
	float Quality;
};
