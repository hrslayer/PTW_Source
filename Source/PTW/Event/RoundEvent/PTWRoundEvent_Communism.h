// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Event/RoundEvent/PTWRoundEventBase.h"
#include "PTWRoundEvent_Communism.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWRoundEvent_Communism : public UPTWRoundEventBase
{
	GENERATED_BODY()

	virtual void OnEventStart(UWorld* World) override;
};
