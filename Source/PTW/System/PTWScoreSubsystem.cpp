// Fill out your copyright notice in the Description page of Project Settings.


#include "System/PTWScoreSubsystem.h"

#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"


void UPTWScoreSubsystem::SavePlayerGameData(const FString& PlayerID, const FPTWPlayerGameData& PlayerGameData)
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	KnownPlayersGameData.Add(PlayerID, PlayerGameData);
}

void UPTWScoreSubsystem::SaveServerTravelPlayerCount(int32 NewPlayerCount)
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	ServerTravelPlayerCount = NewPlayerCount;
}

void UPTWScoreSubsystem::SaveGameData(const FPTWGameData& GameData)
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	SavedGameData = GameData;
}

void UPTWScoreSubsystem::SaveRoleData(const FString& PlayerID, const FPTWRoleData& PlayerRoleData)
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	RoleData.Add(PlayerID, PlayerRoleData);
}

void UPTWScoreSubsystem::AddWinPoint(const FString& PlayerId, int32 Points)
{
	if (Points <= 0) return; 

	FPTWPlayerGameData* Data = KnownPlayersGameData.Find(PlayerId);
	if (!Data) return;

	Data->PlayerData.TotalWinPoints += Points;
}

FPTWPlayerGameData* UPTWScoreSubsystem::FindPlayerGameData(const FString& PlayerId)
{
	return KnownPlayersGameData.Find(PlayerId);
}

void UPTWScoreSubsystem::RemoveTravelPlayersId()
{
	TravelPlayersId.Empty();
}

void UPTWScoreSubsystem::ResetRoleData()
{
	RoleData.Empty();
}

void UPTWScoreSubsystem::SetPlayerGold(FString PlayerId, int32 Gold)
{
	if (FPTWPlayerGameData* Data = KnownPlayersGameData.Find(PlayerId))
	{
		Data->PlayerData.Gold = Gold;
	}
}

void UPTWScoreSubsystem::BeginPlay()
{
	
}

void UPTWScoreSubsystem::AddConnectedPlayerId(const FString& ConnectedPlayerId, const FPTWPlayerGameData& InPlayerGameData)
{
	KnownPlayersGameData.Add(ConnectedPlayerId, InPlayerGameData);
}

void UPTWScoreSubsystem::AddTravelPlayerId(const FString& TravelPlayerId, const FString& PlayerName)
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	
	TravelPlayersId.Add(TravelPlayerId, PlayerName);
}

void UPTWScoreSubsystem::AddLobbyGoldToAllPlayers(int32 RoundClearGold)
{
	for (auto& Pair: KnownPlayersGameData)
	{
		FPTWLobbyItemData& LobbyItemData = Pair.Value.LobbyItemData;
		int32 TotalGold = 0;

		for (int32 i = 0; i < SavedGameData.LastWinnerInfos.Num(); i++)
		{
			if (LobbyItemData.PredictedData.PredictedPlayer == SavedGameData.LastWinnerInfos[i].WinnerId)
			{
				TotalGold += LobbyItemData.PredictedData.RewardAmount;
			}
		}

		for (int32 i = 0; i < LobbyItemData.SavingData.Num(); i++)
		{
			if (LobbyItemData.SavingData[i].TargetRound == SavedGameData.CurrentRound + 1)
			{
				TotalGold += LobbyItemData.SavingData[i].RewardAmount;
			}
		}

		LobbyItemData.PredictedData.PredictedPlayer = nullptr;
		LobbyItemData.SavingData.Empty();
		TotalGold += RoundClearGold;

		Pair.Value.PlayerData.Gold += TotalGold;
	}
}







