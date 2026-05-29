// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Manager/PTWRouletteEventManager.h"

#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameMode/PTWLobbyGameMode.h"
#include "Event/RoundEvent/PTWRoundEventBase.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "Event/RoundEvent/PTWRoundEventDefinition.h"
#include "Event/RoundEvent/PTWRoundEventTable.h"
#include "MiniGame/PTWMiniGameMapRow.h"
#include "System/PTWScoreSubsystem.h"
#include "System/Shop/PTWShopSubsystem.h"


void UPTWRouletteEventManager::StartRouletteSequence()
{
	SelectedRandomMap();
	SelectedRandomEvent();

	StartRoulette();
}

FName UPTWRouletteEventManager::GetMapRowName(FString MapAddress)
{
	if (!MiniGameMapTable) return NAME_None;

	FString TargetMap = MapAddress;
	
	const TMap<FName, uint8*>& RowMap = MiniGameMapTable->GetRowMap();
	
	for (const auto& Pair : RowMap)
	{
		FPTWMiniGameMapRow* Row = reinterpret_cast<FPTWMiniGameMapRow*>(Pair.Value);
        
		if (!Row) continue;
		
		if (Row->Map.GetLongPackageName() == TargetMap)
		{
			return Pair.Key; 
		}
	}
	
	return NAME_None;
}

FPTWMiniGameMapRow* UPTWRouletteEventManager::GetMiniGameMapRow(FName MapRowName)
{
	if (!MiniGameMapTable) return nullptr;
	
	return MiniGameMapTable->FindRow<FPTWMiniGameMapRow>(MapRowName, TEXT("Map"));;
}

void UPTWRouletteEventManager::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		PTWGameState = World->GetGameState<APTWGameState>();
	}
}

void UPTWRouletteEventManager::StartRoulette()
{
	if (!PTWGameState) return;
	
	FPTWRouletteData RouletteData = PTWGameState->GetRouletteData();
	RouletteData.CurrentPhase = EPTWRoulettePhase::StartRoulette;
	RouletteData.RouletteDuration = 5.f;
	PTWGameState->SetRouletteData(RouletteData);

	UWorld* World = GetWorld();
	if (!World) return;
	
	World->GetTimerManager().SetTimer(RouletteTimer, this, &UPTWRouletteEventManager::EndRoulette, 5.f);
}

void UPTWRouletteEventManager::SelectedRandomMap()
{
	if (!PTWGameState) return;
	if (!MiniGameMapTable) return;
	
	TArray<FName> SelectableRowNames = GetSelectableMapRowNames();

	if (SelectableRowNames.Num() == 0) return;
	
	const int32 RandomIndex = FMath::RandRange(0, SelectableRowNames.Num()-1);
	const FName SelectedRowName = SelectableRowNames[RandomIndex];

	const FPTWMiniGameMapRow* Row = MiniGameMapTable->FindRow<FPTWMiniGameMapRow>(SelectedRowName, TEXT("Map"));

	if (Row)
	{
		TravelLevelName = Row->Map.GetLongPackageName();
		MapTag = Row->MiniGameTag;
		GEngine->AddOnScreenDebugMessage(0, 3.f, FColor::Black, Row->DisplayName.ToString(), false);
		AssignRole(Row->MiniGameRoleAsset.LoadSynchronous());
	}
	
	FPTWRouletteData RouletteData = PTWGameState->GetRouletteData();
	RouletteData.MapRowName = SelectedRowName;
	PTWGameState->SetRouletteData(RouletteData); // GameState에 전달
	
	//PTWGameState->AddPlayedMap(SelectedRowName);
}

void UPTWRouletteEventManager::SelectedRandomEvent()
{
	if (!PTWGameState) return;
	if (!LobbyRoundEventTable) return;

	TArray<FName> RowNames = LobbyRoundEventTable->GetRowNames();

	if (RowNames.Num() ==0) return;

	const int32 RandomIndex = FMath::RandRange(0, RowNames.Num()-1);
	const FName SelectedRowName = RowNames[RandomIndex];

	SelectedRoundEventRow = LobbyRoundEventTable->FindRow<FPTWRoundEventRow>(SelectedRowName, TEXT("Round Event"));
	
	if (SelectedRoundEventRow)
	{
		EventTag = SelectedRoundEventRow->RoundEvent->EventTag;
		GEngine->AddOnScreenDebugMessage(0, 3.f, FColor::Black, SelectedRoundEventRow->RoundEvent->EventName.ToString());
	}
	
	FPTWRouletteData RouletteData = PTWGameState->GetRouletteData();
	RouletteData.EventRowName = SelectedRowName;
	PTWGameState->SetRouletteData(RouletteData);
}

void UPTWRouletteEventManager::AssignRole(UPTWMiniGameRoleAsset* RoleAsset)
{
	if (!RoleAsset) return;
	
	TArray<APlayerState*> Players;

	for (APlayerState* Player : PTWGameState->PlayerArray)
	{
		Players.Add(Player);
	}

	for (int32 i = Players.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		Players.Swap(i, j);
	}

	TArray<TPair<FPTWRoleData, int32>> RoleCount;
	int32 RemainingPlayers = Players.Num();
	
	// 고정 인원 역할 
	for (const FPTWRoleRatio& Ratio : RoleAsset->RoleRatios)
	{
		if (Ratio.CountMode == EPTWRoleCountMode::Fixed)
		{
			for (const FPTWRoleData& RoleData : RoleAsset->RoleData)
			{
				if (RoleData.RoleTag == Ratio.Role)
				{
					RoleCount.Add({ RoleData, Ratio.Count });
					RemainingPlayers -= Ratio.Count;
				}
			}
		}
	}

	// 비율 인원 역할
	int32 TotalRatio = 0;
	const FPTWRoleRatio* LastRatioRole = nullptr;
	for (const FPTWRoleRatio& Ratio : RoleAsset->RoleRatios)
	{
		if (Ratio.CountMode == EPTWRoleCountMode::Ratio)
		{
			TotalRatio += Ratio.Count;
			LastRatioRole = &Ratio;
		}
	}

	int32 RatioAssigned = 0;
	for (const FPTWRoleRatio& Ratio : RoleAsset->RoleRatios)
	{
		if (Ratio.CountMode == EPTWRoleCountMode::Ratio)
		{
			for (const FPTWRoleData& RoleData : RoleAsset->RoleData)
			{
				if (RoleData.RoleTag == Ratio.Role)
				{
					int32 Count = (&Ratio == LastRatioRole) ? RemainingPlayers - RatioAssigned :
					FMath::RoundToInt((float)RemainingPlayers * Ratio.Count / TotalRatio);

					Count = FMath::Max(Count, 1);
					
					RatioAssigned += Count;
					RoleCount.Add({ RoleData, Count });
				}
			}
		}
	}
	
	// 고정 인원 외 나머지 인원 역할 배정
	for (const FPTWRoleRatio& Ratio : RoleAsset->RoleRatios)
	{
		if (Ratio.CountMode == EPTWRoleCountMode::Remainder)
		{
			for (const FPTWRoleData& RoleData : RoleAsset->RoleData)
			{
				if (RoleData.RoleTag == Ratio.Role)
				{
					RoleCount.Add({ RoleData, RemainingPlayers });
				}
			}
		}
	}

	int32 PlayerIndex = 0;
	for (auto& Count : RoleCount)
	{
		for (int32 i =0; i < Count.Value; i++)
		{
			if (!Players.IsValidIndex(PlayerIndex)) return;
			
			APTWPlayerState* PlayerState = Cast<APTWPlayerState>(Players[PlayerIndex]);
			if (PlayerState)
			{
				PlayerState->SetRoleData(Count.Key);

				if (UPTWScoreSubsystem* ScoreSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
				{
					ScoreSubsystem->SaveRoleData(PlayerState->GetUniqueId().ToString(), Count.Key);
				}
			}

			PlayerIndex++;
		}
	}
}

TArray<FName> UPTWRouletteEventManager::GetSelectableMapRowNames()
{
	TArray<FName> SelectableRowNames;
	
	if (!PTWGameState)
	{
		return SelectableRowNames;
	}
	if (!MiniGameMapTable)
	{
		return SelectableRowNames;
	}

	int32 PlayerCount = PTWGameState->PlayerArray.Num();
	TArray<FName> RowNames = MiniGameMapTable->GetRowNames();

	SelectableRowNames.Reserve(RowNames.Num());

	// 모든 Row를 순회하면서 선택 가능 여부를 검사
	for (const FName& RowName : RowNames)
	{
		const FPTWMiniGameMapRow* Row = MiniGameMapTable->FindRow<FPTWMiniGameMapRow>(RowName, TEXT("Map"));
		if (!Row) continue;

		//맵 중복 방지
		if (PTWGameState->GameData.PlayedMapRowNames.Contains(RowName)) continue;
		
		if (Row->MinPlayers <= PlayerCount && Row->MaxPlayers >= PlayerCount)
		{
			// 조건을 만족하면 선택 가능한 후보로 추가
			SelectableRowNames.Add(RowName);
		}
	}
	// 조건을 만족하는 모든 맵 RowName 반환
	return SelectableRowNames;
}

void UPTWRouletteEventManager::EndRoulette()
{
	if (!PTWGameState) return;
	
	FPTWRouletteData RouletteData = PTWGameState->GetRouletteData();
	RouletteData.CurrentPhase = EPTWRoulettePhase::Finished;
	PTWGameState->SetRouletteData(RouletteData);
	
	if (UPTWShopSubsystem* ShopSubsystem = GetWorld()->GetSubsystem<UPTWShopSubsystem>())
	{
		ShopSubsystem->InitializeShopsForRound(MapTag, EventTag);
	}

	FName SelectedMapRowName = PTWGameState->GetRouletteData().MapRowName;

	
	UWorld* World = GetWorld();
	if (!World) return;

	UPTWRoundEventBase* Event = NewObject<UPTWRoundEventBase>(this, SelectedRoundEventRow->RoundEvent->EventLogic);
	Event->OnEventStart(GetWorld());
	
	OnRouletteFinished.Broadcast(SelectedMapRowName);
}
