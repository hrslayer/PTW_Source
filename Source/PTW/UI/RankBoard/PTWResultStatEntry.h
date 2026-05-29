// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWResultStatEntry.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class PTW_API UPTWResultStatEntry : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetStatData(const FText& InName, float InValue);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatValueText;
};
