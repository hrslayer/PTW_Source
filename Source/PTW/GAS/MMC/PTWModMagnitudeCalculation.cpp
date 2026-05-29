// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWModMagnitudeCalculation.h"

UPTWModMagnitudeCalculation::UPTWModMagnitudeCalculation()
{
	RelevantAttributesToCapture.Add(FPTWDamageStatics::DamageStatics().WeaponDamageDef);
	RelevantAttributesToCapture.Add(FPTWDamageStatics::DamageStatics().DefenseDef);
}

float UPTWModMagnitudeCalculation::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	float WeaponDamage = 0.0f;
	GetCapturedAttributeMagnitude(FPTWDamageStatics::DamageStatics().WeaponDamageDef, Spec, FAggregatorEvaluateParameters(), WeaponDamage);
	
	float FinalDamage = GetDamageAfterDefense(WeaponDamage, Spec);
    
	return FMath::Max<float>(FinalDamage, 1.0f);
}
