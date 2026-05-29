// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWGA_Fire.h"
#include "PTWGA_ProjectileFire.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_ProjectileFire : public UPTWGA_Fire
{
	GENERATED_BODY()
	
protected:
	virtual void PerformFireAction(const FPTWFireConext Context) override;
};
