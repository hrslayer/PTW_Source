// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/PTWGA_ProjectileFire.h"
#include "PTWGA_KnockBackProjectileFire.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_KnockBackProjectileFire : public UPTWGA_ProjectileFire
{
	GENERATED_BODY()
	
protected:
	virtual void PerformFireAction(const FPTWFireConext Context) override;
	virtual void ApplyKnockBack(const FPTWFireConext& Context);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float LaunchDirStrength = 1250.0f;
};
