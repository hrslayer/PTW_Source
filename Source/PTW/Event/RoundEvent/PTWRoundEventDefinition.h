// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PTWRoundEventDefinition.generated.h"

class UPTWRoundEventBase;
/**
 * 
 */
UCLASS()
class PTW_API UPTWRoundEventDefinition : public UDataAsset
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere)
	FText EventName;
	
	UPROPERTY(EditAnywhere)
	FGameplayTag EventTag;
	
	UPROPERTY(EditAnywhere)
	FText EventDescription;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UPTWRoundEventBase> EventLogic;
};

