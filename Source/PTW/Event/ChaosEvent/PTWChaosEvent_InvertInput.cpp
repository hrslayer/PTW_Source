// Fill out your copyright notice in the Description page of Project Settings.


#include "Event/ChaosEvent/PTWChaosEvent_InvertInput.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/Character/Component/PTWChaosAffectedComponent.h"
#include "GameFramework/PlayerState.h"

void UPTWChaosEvent_InvertInput::ApplyEvent(APTWGameState* GameState)
{
	Super::ApplyEvent(GameState);
	
	if (!GameState) return;

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (APTWPlayerCharacter* Character = Cast<APTWPlayerCharacter>(PlayerState->GetPawn()))
		{
			Character->GetChaosAffectedComponent()->Client_SetInputState(false, true);
		}
	}
}

void UPTWChaosEvent_InvertInput::EndEvent(APTWGameState* GameState)
{
	Super::EndEvent(GameState);
	if (!GameState) return;
	
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (APTWPlayerCharacter* Character = Cast<APTWPlayerCharacter>(PlayerState->GetPawn()))
		{
			Character->GetChaosAffectedComponent()->Client_SetInputState(false, false);
		}
	}
	
}
