// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWBaseDamageMMC.h"
#include "PTWExplosionMMC.generated.h"

/**
 * 
 */ 
UCLASS()
class PTW_API UPTWExplosionMMC : public UPTWBaseDamageMMC
{
	GENERATED_BODY()
	
public:
	UPTWExplosionMMC();
	
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
