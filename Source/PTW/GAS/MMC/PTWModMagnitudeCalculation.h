// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "PTWBaseDamageMMC.h"
#include "../PTWAbilityBattleAttributeSet.h"
#include "../PTWAttributeSet.h"
#include "../PTWWeaponAttributeSet.h"
#include "PTWModMagnitudeCalculation.generated.h"



/**
 * 
 */
UCLASS()
class PTW_API UPTWModMagnitudeCalculation : public UPTWBaseDamageMMC
{
	GENERATED_BODY()
	
public:
	UPTWModMagnitudeCalculation();
	
	float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
