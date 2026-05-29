// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/PTWMiniGameMapRow.h"
#include "MiniGame/Data/PTWResultStateData.h"
#include "PTWGameData.generated.h"

/**
 * 
 */
USTRUCT(Blueprintable)
struct FPTWChaosItemEntry 
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName ItemId;

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;
};

USTRUCT(Blueprintable)
struct FPTWLastWinnerInfo 
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString WinnerId;
	
};

USTRUCT(Blueprintable)
struct FPTWGameData 
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentRound;

	UPROPERTY(BlueprintReadOnly)
	float ScoreRadio = 1.f;
	
	// 미니 게임이 끝나면 데이터 삭제
	UPROPERTY(BlueprintReadOnly)
	TArray<FPTWChaosItemEntry> ChaosItemEntries;

	UPROPERTY(BlueprintReadOnly)
	FPTWMiniGameMapRow CurrentMiniGameMapRow;
	
	UPROPERTY(BlueprintReadOnly)
	TSet<FName> PlayedMapRowNames;

	UPROPERTY(BlueprintReadOnly)
	TArray<FPTWLastWinnerInfo> LastWinnerInfos;
};

USTRUCT(Blueprintable)
struct FPTWBaseRankingData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FString PlayerId;
	
	UPROPERTY(BlueprintReadOnly)
	int32 Rank =0;
	
	UPROPERTY(BlueprintReadOnly)
	FString PlayerName = "";
	
	UPROPERTY(BlueprintReadOnly)
	int32 Score = 0;
	
	UPROPERTY(BlueprintReadWrite)
	TArray<FString> InventoryItemIDs;

	UPROPERTY(BlueprintReadOnly)
	bool bLeftGame;
};

USTRUCT(Blueprintable)
struct FPTWLobbyRankingData : public FPTWBaseRankingData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	int32 Gold;
	
};

USTRUCT(Blueprintable)
struct FPTWMiniGameRankingData : public FPTWBaseRankingData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	int32 Kill = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Death = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 DeathOrder = -1;

	UPROPERTY(BlueprintReadOnly)
	int32 AwardedPoints  = 0;
	
	UPROPERTY(BlueprintReadOnly)
	int32 TeamId  = -1;
	
};

USTRUCT(Blueprintable)
struct FPTWMiniGameResultStats
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	TMap<EPTWResultStatName, float> ResultStats; 
};

USTRUCT(Blueprintable)
struct FPTWMiniGameResultStatData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FText StatName;
	
	UPROPERTY(BlueprintReadOnly)
	float StatValue;
};

USTRUCT(Blueprintable)
struct FPTWMiniGameResultData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly)
	TArray<FPTWMiniGameResultStatData> StatData;
};

USTRUCT(Blueprintable)
struct FPTWMiniGameTopResultData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> PlayerNames;

	UPROPERTY(BlueprintReadOnly)
	FPTWMiniGameResultStatData StatData;
};


