// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PTWChaosEventBase.generated.h"

class APTWGameState;
/**
 * 
 */
UCLASS()
class PTW_API UPTWChaosEventBase : public UObject
{
	GENERATED_BODY()
public:
	virtual void ApplyEvent(APTWGameState* GameState);
	virtual void EndEvent(APTWGameState* GameState);
};
