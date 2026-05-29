// Fill out your copyright notice in the Description page of Project Settings.


#include "Event/ChaosEvent/PTWChaosEvent_LoserRevival.h"

#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "MiniGame/PTWMiniGameMode.h"

void UPTWChaosEvent_LoserRevival::ApplyEvent(APTWGameState* GameState)
{
	APTWMiniGameMode* GameMode = GameState->GetWorld()->GetAuthGameMode<APTWMiniGameMode>();
	if (!GameMode) return;

	GameMode->ReviveAllDeadPlayers();
}
