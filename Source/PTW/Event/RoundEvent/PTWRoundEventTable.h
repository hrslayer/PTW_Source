// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PTWRoundEventTable.generated.h"


class UPTWRoundEventDefinition;

USTRUCT(BlueprintType)
struct FPTWRoundEventRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UPTWRoundEventDefinition> RoundEvent;
};
