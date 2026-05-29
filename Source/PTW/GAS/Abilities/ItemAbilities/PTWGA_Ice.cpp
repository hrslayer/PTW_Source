// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Ice.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "TimerManager.h"

void UPTWGA_Ice::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                        const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UPTWGA_Ice::ApplyItemEffect()
{
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PC) return;

	IPTWCombatInterface* CombatInterface = Cast<IPTWCombatInterface>(PC);
	if (!CombatInterface) return;

	CombatInterface->ApplyGameplayEffectToSelf(
		EffectClass,
		1.0f,
		FGameplayEffectContextHandle()
	);

	PC->SetIceVisual(true);

	FTimerHandle IceVisualTimerHandle;
	PC->GetWorldTimerManager().SetTimer(
		IceVisualTimerHandle,
		FTimerDelegate::CreateWeakLambda(PC, [PC]()
		{
			if (IsValid(PC))
			{
				PC->SetIceVisual(false);
			}
		}),
		3.0f,
		false
	);
}
