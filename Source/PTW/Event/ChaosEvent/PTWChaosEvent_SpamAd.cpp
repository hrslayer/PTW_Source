// Fill out your copyright notice in the Description page of Project Settings.


#include "Event/ChaosEvent/PTWChaosEvent_SpamAd.h"

#include "CoreFramework/PTWPlayerController.h"
#include "GameFramework/PlayerState.h"

void UPTWChaosEvent_SpamAd::ApplyEvent(APTWGameState* GameState)
{
	if (!GameState) return;
	
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (APTWPlayerController* PlayerController = Cast<APTWPlayerController>(PlayerState->GetPlayerController()))
		{
			PlayerController->Client_SetSpamAd(true);
		}
	}
}

void UPTWChaosEvent_SpamAd::EndEvent(APTWGameState* GameState)
{
	if (!GameState) return;
	
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (APTWPlayerController* PlayerController = Cast<APTWPlayerController>(PlayerState->GetPlayerController()))
		{
			PlayerController->Client_SetSpamAd(false);
		}
	}
}
