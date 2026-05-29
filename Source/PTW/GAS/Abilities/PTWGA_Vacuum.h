// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/PTWGA_Fire.h"
#include "PTWGA_Vacuum.generated.h"

/**
 * 
 */
class UPTWVacuumWeaponData;

UCLASS()
class PTW_API UPTWGA_Vacuum : public UPTWGA_Fire
{
	GENERATED_BODY()
	
public:
	UPTWGA_Vacuum();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void StartFire() override;
	virtual void StopFire() override;
	virtual void AutoFire() override;

	void ApplySuction(const UPTWVacuumWeaponData* VacuumData);

	void StartOverheat(float Duration);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vacuum|Regen")
	TSubclassOf<UGameplayEffect> RegenGameplayEffectClass;

	FTimerHandle OverheatTimerHandle;

	FActiveGameplayEffectHandle RegenEffectHandle;
};
