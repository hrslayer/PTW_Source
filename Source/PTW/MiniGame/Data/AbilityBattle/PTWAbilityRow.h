// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWTierStyleAsset.h"
#include "PTWAbilityRow.generated.h"

class UPTWAbilityDefinition;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FPTWAbilityRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EPTWAbilityTier Tier;
	
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UPTWAbilityDefinition> AbilityDefinition;
};
