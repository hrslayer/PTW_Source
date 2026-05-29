// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PTWRoundEventBase.generated.h"

/**
 * 
 */
UCLASS(DefaultToInstanced)
class PTW_API UPTWRoundEventBase : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnEventStart(UWorld* World);
};
