// Fill out your copyright notice in the Description page of Project Settings.


#include "Event/RoundEvent/PTWRoundEvent_DoubleChance.h"

#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "System/PTWScoreSubsystem.h"

void UPTWRoundEvent_DoubleChance::OnEventStart(UWorld* World)
{
	APTWGameState* GameState = World->GetGameState<APTWGameState>();

	if (!GameState) return;

	GameState->GameData.ScoreRadio = 2.f;
}
