// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "GAS/PTWAttributeSet.h"
#include "GAS/PTWWeaponAttributeSet.h"
#include "PTWBaseDamageMMC.generated.h"

struct FPTWDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition WeaponDamageDef;
	FGameplayEffectAttributeCaptureDefinition DefenseDef;
	
	FPTWDamageStatics()
	{
		WeaponDamageDef = FGameplayEffectAttributeCaptureDefinition(
			UPTWWeaponAttributeSet::GetDamageAttribute(), 
			EGameplayEffectAttributeCaptureSource::Source, 
			false);
		
		DefenseDef = FGameplayEffectAttributeCaptureDefinition(
			UPTWAttributeSet::GetDefenseAttribute(),
			EGameplayEffectAttributeCaptureSource::Target, 
			false);
	}
	
	static const FPTWDamageStatics& DamageStatics() { static FPTWDamageStatics DStatics; return DStatics; }
};

/**
 * 
 */
UCLASS()
class PTW_API UPTWBaseDamageMMC : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
	
protected:
	float GetDamageAfterDefense(float RawDamage, const FGameplayEffectSpec& Spec) const;
};
