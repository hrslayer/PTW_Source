// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWRocketGameMode.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "GAS/PTWAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "MiniGame/Actor/Rocket/PTWDropWarningLamp.h"
#include "MiniGame/Manager/Rocket/PTWCollapseManager.h"
#include "System/PTWItemSpawnManager.h"

APTWRocketGameMode::APTWRocketGameMode()
{
	SetMiniGameRule();
}

void APTWRocketGameMode::StartRound()
{
	GiveRocketWeapon();
	StartCollapseTimer();
	SetAlivePlayerStates();
	Super::StartRound();
}

void APTWRocketGameMode::SetMiniGameRule()
{
	MiniGameRule.SpawnRule.bUseRespawn = false;
	MiniGameRule.TimeRule.Timer = 120.0f;
	MiniGameRule.WinConditionRule.WinType = EPTWWinType::Survival;
	MiniGameRule.SpawnRule.RespawnDelay = 1.0f;
}

void APTWRocketGameMode::GiveRocketWeapon()
{
	APTWGameState* GS_Ref = GetWorld()->GetGameState<APTWGameState>();
	
	if (!GS_Ref) return;
	
	UPTWItemSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>();
	if (!SpawnManager) return;
	
	for (const TObjectPtr<APlayerState>& PS_Ptr : GS_Ref->PlayerArray)
	{
		APTWPlayerState* PTWPS = Cast<APTWPlayerState>(PS_Ptr);
    
		if (PTWPS)
		{
			SpawnManager->SpawnSingleItem(PTWPS, RocketDefinition);
		}
	}
}

void APTWRocketGameMode::RestartPlayer(AController* NewPlayer)
{
	if (!NewPlayer) return;
	
	if (APlayerState* PS = NewPlayer->PlayerState)
	{
		if (DeadPlayerStates.Contains(PS))
		{
			SetSpectatorPawn(NewPlayer);
			return;
		}
	} 
	
	Super::RestartPlayer(NewPlayer);
}

void APTWRocketGameMode::HandlePlayerDeath(AActor* DeadActor, AActor* KillActor)
{
	Super::HandlePlayerDeath(DeadActor, KillActor);
	
	APTWPlayerCharacter* DeadCharacter = Cast<APTWPlayerCharacter>(DeadActor);
	if (!DeadCharacter) return;
	
	APTWPlayerState* DeadController = Cast<APTWPlayerState>(DeadCharacter->GetPlayerState());
	
	if (!DeadController) return;
	DeadPlayerStates.Add(DeadController);
}

void APTWRocketGameMode::SetSpectatorPawn(AController* Controller)
{
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!PC) return;
	
	PC->ChangeState(NAME_Spectating);
	PC->StartSpectatingOnly();
}

void APTWRocketGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	CollaspeManager = Cast<APTWCollapseManager>(UGameplayStatics::GetActorOfClass(GetWorld(), APTWCollapseManager::StaticClass()));
	WarningLamp = Cast<APTWDropWarningLamp>(UGameplayStatics::GetActorOfClass(GetWorld(), APTWDropWarningLamp::StaticClass()));
}

void APTWRocketGameMode::ExecuteCollapseSequence()
{
	if (CollaspeManager)
	{
		int32 TileSize = GetCollapseTiles();
		if (TileSize <= 0)
		{
			StopCollapseSequence();
			return;
		}
		
		CollaspeManager->ExecuteCollapseSequence(GetRandomIndex(TileSize));
	}
	if (WarningLamp)
	{
		WarningLamp->StartDropWarningLamp();
	}
}

void APTWRocketGameMode::StartDropWarning()
{
	if (WarningLamp)
	{
		WarningLamp->MulticastRPC_StartWarningLight();
	}
}

int32 APTWRocketGameMode::GetCollapseTiles()
{
	if (CollaspeManager)
	{
		return CollaspeManager->GetTileGroupParentsSize();
	}
	
	return 0;
}

void APTWRocketGameMode::SetAlivePlayerStates()
{
	OutAlive.Reset();
	
	APTWGameState* PTWGS = GetGameState<APTWGameState>();
	if (!PTWGS) return;
	
	for (APlayerState* PS : PTWGS->PlayerArray)
	{
		APTWPlayerState* PTWPS = Cast<APTWPlayerState>(PS);
		if (!PTWPS) continue;
		
		OutAlive.Add(PTWPS);
		
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PS);
		if (!ASC) continue;
		
		const UPTWAttributeSet* AttributeSet = Cast<UPTWAttributeSet>(ASC->GetAttributeSet(UPTWAttributeSet::StaticClass()));
		if (!AttributeSet) continue;
		
		UPTWAttributeSet* PTWAS = const_cast<UPTWAttributeSet*>(AttributeSet);
		
		PTWAS->OnKillApplied.AddUObject(this, &APTWRocketGameMode::KillPlayerPossess);
	}
	
	
	AlivePlayerCounts = OutAlive.Num();
}

int32 APTWRocketGameMode::GetRandomIndex(int32 TileSize)
{
	if (TileSize <= 0) return 0;
	return FMath::RandRange(0, TileSize - 1);
}

void APTWRocketGameMode::StopCollapseSequence()
{
	GetWorld()->GetTimerManager().ClearTimer(CollapseTimer);
}

void APTWRocketGameMode::StopDropWarning()
{
	if (WarningLamp)
	{
		WarningLamp->MulticastRPC_StopWarningLight();
	}
}

void APTWRocketGameMode::KillPlayerPossess(AActor* KillActor)
{
	if (APTWPlayerState* PTWPS = Cast<APTWPlayerState>(KillActor))
	{
		int32 KillCount = PTWPS->GetPlayerRoundData().KillCount + 1;
		const FString& UniqueID = PTWPS->GetUniqueId().ToString();
		
		AddResultDataValue(UniqueID, EPTWResultStatName::Rocket_Kill, KillCount);
	}
}

void APTWRocketGameMode::StartCollapseTimer()
{
	GetWorld()->GetTimerManager().SetTimer(CollapseTimer, this, &APTWRocketGameMode::ExecuteCollapseSequence, 10.0f,true);
}
