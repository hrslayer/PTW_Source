// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWGA_ItemAbilityBase.h"
#include "PTWGA_Grenade.generated.h"

class AGrenadeActor;
/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_Grenade : public UPTWGA_ItemAbilityBase
{
	GENERATED_BODY()
	
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InitializeVariable() override;
	virtual void ApplyItemEffect() override;
	
	float GetGrenadeBaseDamage(APTWPlayerCharacter* PC);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawn")
	TSubclassOf<AGrenadeActor> GrenadeActor;
};
