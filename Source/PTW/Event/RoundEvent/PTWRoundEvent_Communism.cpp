// Fill out your copyright notice in the Description page of Project Settings.


#include "Event/RoundEvent/PTWRoundEvent_Communism.h"

#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "System/PTWScoreSubsystem.h"

void UPTWRoundEvent_Communism::OnEventStart(UWorld* World)
{
	if (!World) return;

	APTWGameState* GameState = World->GetGameState<APTWGameState>();
	UPTWScoreSubsystem* ScoreSubsystem = World->GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>();

	if (!GameState || !ScoreSubsystem) return;

	TMap<FString, APTWPlayerState*> Players;
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState))
		{
			Players.Add(PlayerState->GetUniqueId().ToString(), PTWPlayerState);
		}
	}

	int32 TotalGold = 0;
	int32 PlayerCount = 0;

	for (auto& [PlayerId, SavedPlayerData] : ScoreSubsystem->GetKnownPlayersGameData())
	{
		if (APTWPlayerState** PlayerState = Players.Find(PlayerId))
		{
			TotalGold += (*PlayerState)->GetPlayerData().Gold;
		}
		else
		{
			TotalGold += SavedPlayerData.PlayerData.Gold;
		}
		PlayerCount++;
	}

	if (PlayerCount == 0) return;
	int32 SharedGold = TotalGold / PlayerCount;

	for (auto& [PlayerId, SavedPlayerData] : ScoreSubsystem->GetKnownPlayersGameData())
	{
		if (APTWPlayerState** PlayerState = Players.Find(PlayerId))
		{
			FPTWPlayerData PlayerData = (*PlayerState)->GetPlayerData();
			PlayerData.Gold = SharedGold;
			
			(*PlayerState)->SetPlayerData(PlayerData);
		}
		else
		{
			ScoreSubsystem->SetPlayerGold(PlayerId, SharedGold);
		}
	}
}
