// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWGA_Fire.h"
#include "PTWGA_ShotGunFire.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_ShotGunFire : public UPTWGA_Fire
{
	GENERATED_BODY()
	
protected:
	virtual void HandleHitScan(const FPTWFireConext Context);
	virtual void ApplyDamageToTarget(const FGameplayAbilityTargetDataHandle& TargetData, float BaseDamage);
	void PerformLineTraceShotGun(TArray<FHitResult>& OutHitResult, APTWPlayerCharacter* PlayerCharacter);
	void HitAction(const FHitResult* HitResult);
};
