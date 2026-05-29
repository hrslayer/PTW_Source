// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Manager/PTWChaosEventApply.h"

#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "EngineUtils.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "Event/PTWChaosItemDefinition.h"
#include "Event/ChaosEvent/PTWChaosEventBase.h"


void UPTWChaosEventApply::InitDefinition(UPTWChaosItemDefinition* InDefinition)
{
	if (!InDefinition) return;
	
	Definition = InDefinition;
}

void UPTWChaosEventApply::SetStackCount(int32 Count)
{
	StackCount = Count;
}


void UPTWChaosEventApply::ApplyChaosEvent(APTWGameState* GameState)
{
	if (!GameState || !Definition) return;
	
	if (Definition->ChaosEventClass)
	{
		UPTWChaosEventBase* CurrentEventClass = NewObject<UPTWChaosEventBase>(this,Definition->ChaosEventClass);
		CurrentEventClass->ApplyEvent(GameState);
	}
	if (Definition->ChaosAbilityClass)
	{
		ApplyChaosAbilityClass(GameState);
	}
	if (!Definition->ChaosWeatherDA.IsNull())  
	{
		ApplyChaosWeather(GameState);
	}
	
	GameState->AddCurrentChaosEvent(Definition->ChaosEventTag);
}

void UPTWChaosEventApply::ChaosEventEnd(APTWGameState* GameState)
{
	if (Definition->ChaosEventClass)
	{
		UPTWChaosEventBase* CurrentEventClass = NewObject<UPTWChaosEventBase>(this,Definition->ChaosEventClass);
		CurrentEventClass->EndEvent(GameState);
	}
	
	ChaosAbilityEnd();
	ChaosWeatherEnd(GameState);

	GameState->ResetCurrentChaosEvent();
}

void UPTWChaosEventApply::ApplyChaosAbilityClass(APTWGameState* GameState)
{
	if (!IsValid(Definition) || !Definition->ChaosAbilityClass) return;

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		UAbilitySystemComponent* ASC = nullptr;
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerState))
		{
			ASC = ASI->GetAbilitySystemComponent();
		}
		if (!IsValid(ASC)) continue;

		FGameplayAbilitySpec Spec(Definition->ChaosAbilityClass, StackCount);
		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
		ASC->TryActivateAbility(Handle);
        
		ApplyAbilityHandles.Add(ASC, Handle);
	}
}

void UPTWChaosEventApply::ChaosAbilityEnd()
{
	for (auto& Pair : ApplyAbilityHandles)
	{
		if (!IsValid(Pair.Key)) continue;
		
		Pair.Key->CancelAbilityHandle(Pair.Value);
		Pair.Key->ClearAbility(Pair.Value); 
	}
	ApplyAbilityHandles.Empty();
}

void UPTWChaosEventApply::ApplyChaosWeather(APTWGameState* GameState)
{
	if (!IsValid(Definition) || Definition->ChaosWeatherDA.IsNull()) return;

	AActor* Sky = GameState->SkyActor;
	if (!IsValid(Sky)) return;

	UFunction* GetWeatherFunc = Sky->FindFunction(TEXT("GetCurrentWeather"));
	if (!GetWeatherFunc) return;

	struct { UObject* ReturnValue; } GetParams{};
	Sky->ProcessEvent(GetWeatherFunc, &GetParams);
	PreviousWeather = GetParams.ReturnValue;

	UObject* WeatherDA = Definition->ChaosWeatherDA.LoadSynchronous();
	if (!WeatherDA) return;

	UFunction* SetWeatherFunc = Sky->FindFunction(TEXT("SetWeather"));
	if (!SetWeatherFunc) return;
	
	struct { UObject* WeatherDA; } SetParams{ WeatherDA };
	Sky->ProcessEvent(SetWeatherFunc, &SetParams);
}

void UPTWChaosEventApply::ChaosWeatherEnd(APTWGameState* GameState)
{
	AActor* Sky = GameState->SkyActor;
	if (!IsValid(Sky)) return;
	
	UFunction* SetWeatherFunc = Sky->FindFunction(TEXT("SetWeather"));
	if (!SetWeatherFunc) return;
	
	struct { UObject* WeatherDA; } SetParams{ PreviousWeather };
	Sky->ProcessEvent(SetWeatherFunc, &SetParams);
}



