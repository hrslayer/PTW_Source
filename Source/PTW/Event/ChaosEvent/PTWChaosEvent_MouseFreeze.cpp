// Fill out your copyright notice in the Description page of Project Settings.


#include "Event/ChaosEvent/PTWChaosEvent_MouseFreeze.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/Character/Component/PTWChaosAffectedComponent.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "GameFramework/PlayerState.h"

class APTWPlayerController;

void UPTWChaosEvent_MouseFreeze::ApplyEvent(APTWGameState* GameState)
{
	Super::ApplyEvent(GameState);

	if (!GameState) return;
	
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (APTWPlayerCharacter* Character = Cast<APTWPlayerCharacter>(PlayerState->GetPawn()))
		{
			Character->GetChaosAffectedComponent()->Client_SetInputState(true, false);
		}
	}
}

void UPTWChaosEvent_MouseFreeze::EndEvent(APTWGameState* GameState)
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
