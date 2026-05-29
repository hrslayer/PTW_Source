// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Event/ChaosEvent/PTWChaosEventBase.h"
#include "PTWChaosEvent_SlowDown.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWChaosEvent_SlowDown : public UPTWChaosEventBase
{
	GENERATED_BODY()

public:
	virtual void ApplyEvent(APTWGameState* GameState) override;
	virtual void EndEvent(APTWGameState* GameState) override;
};
