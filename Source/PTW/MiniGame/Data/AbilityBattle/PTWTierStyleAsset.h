// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PTWTierStyleAsset.generated.h"

UENUM(BlueprintType)
enum class EPTWAbilityTier : uint8
{
	Bronze,
	Silver,
	Gold
};

USTRUCT(BlueprintType)
struct FPTWTierData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	FLinearColor TierBorderColor;
	
	UPROPERTY(EditDefaultsOnly)
	float TierRatio;
};

UCLASS()
class PTW_API UPTWTierStyleAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TMap<EPTWAbilityTier, FPTWTierData> TierData;
};
