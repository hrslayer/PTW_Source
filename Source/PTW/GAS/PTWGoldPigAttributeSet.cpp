// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGoldPigAttributeSet.h"

UPTWGoldPigAttributeSet::UPTWGoldPigAttributeSet()
{
}

void UPTWGoldPigAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UPTWGoldPigAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

void UPTWGoldPigAttributeSet::OnRep_PigHealth(const FGameplayAttributeData& OldPigHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWGoldPigAttributeSet, PigHealth, OldPigHealth);
}
