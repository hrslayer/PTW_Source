// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Event/RoundEvent/PTWRoundEventBase.h"
#include "PTWRoundEvent_DoubleChance.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWRoundEvent_DoubleChance : public UPTWRoundEventBase
{
	GENERATED_BODY()

public:
	virtual void OnEventStart(UWorld* World) override;
	
};
