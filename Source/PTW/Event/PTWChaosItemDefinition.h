// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PTWChaosItemDefinition.generated.h"

/**
 * 
 */


class UPTWChaosEventBase;
class UGameplayAbility;
class UGameplayEffect;

UCLASS(BlueprintType)
class PTW_API UPTWChaosItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Event")
	FName ItemId;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Play")
	bool bUseStack;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag ChaosEventTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS")
	TSubclassOf<UGameplayAbility> ChaosAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Class")
	TSubclassOf<UPTWChaosEventBase> ChaosEventClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weather")
	TSoftObjectPtr<UObject> ChaosWeatherDA;
	
};
