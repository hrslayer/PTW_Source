// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWAttributeSet.h"
#include "PTWGoldPigAttributeSet.generated.h"

/**
 * 
 */

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


UCLASS()
class PTW_API UPTWGoldPigAttributeSet : public UPTWAttributeSet
{
	GENERATED_BODY()
	
public:
	UPTWGoldPigAttributeSet();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PigHealth, Category = "GoldPig|Attributes")
	FGameplayAttributeData PigHealth;
	ATTRIBUTE_ACCESSORS(UPTWGoldPigAttributeSet, PigHealth);
	
protected:
	UFUNCTION()
	void OnRep_PigHealth(const FGameplayAttributeData& OldPigHealth);
	
};
