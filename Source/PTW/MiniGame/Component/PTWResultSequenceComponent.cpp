// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Component/PTWResultSequenceComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "Gameplay/Actor/PTWResultCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "MiniGame/PTWMiniGameMode.h"

UPTWResultSequenceComponent::UPTWResultSequenceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPTWResultSequenceComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UPTWResultSequenceComponent::StartResultSequence()
{
	UWorld* World = GetWorld();
	if (!World) return;
	if (!GameState) return;

	if (GameState->GetCurrentGamePhase() == EPTWGamePhase::MiniGame)
	{
		GameState->SetCurrentPhase(EPTWGamePhase::MiniGameResult);
	}
	else if (GameState->GetCurrentGamePhase() == EPTWGamePhase::Lobby)
	{
		GameState->SetCurrentPhase(EPTWGamePhase::GameResult);
	}

	AActor* ResultCamera = nullptr;
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(World, FName("ResultCamera"), FoundActors);
	if (FoundActors.Num() > 0) ResultCamera = FoundActors[0];

	TArray<AActor*> WinSpots;
	UGameplayStatics::GetAllActorsWithTag(World, FName("WinSpot"), WinSpots);

	TArray<AActor*> LoseSpots;
	UGameplayStatics::GetAllActorsWithTag(World, FName("LoseSpot"), LoseSpots);

	int32 CurrentWinIndex = 0;
	int32 CurrentLoseIndex = 0;

	TArray<FPTWLastWinnerInfo>& LastWinnerInfo = GameState->GameData.LastWinnerInfos;
	LastWinnerInfo.Empty();
	
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get());
		if (!PC) continue;

		APTWPlayerState* PS = PC->GetPlayerState<APTWPlayerState>();
		if (!PS) continue;

		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);

		if (APawn* OriginalPawn = PC->GetPawn())
		{
			OriginalPawn->SetActorHiddenInGame(true);
			OriginalPawn->SetActorEnableCollision(false);
		}

		if (ResultCamera)
		{
			FViewTargetTransitionParams Params;
			Params.BlendTime = 1.0f;
			Params.BlendFunction = EViewTargetBlendFunction::VTBlend_Linear;
			Params.bLockOutgoing = true;

			PC->ClientSetViewTarget(ResultCamera, Params);
		}

		if (ResultCharacterClass)
		{
			bool bIsWinner = IsWinner(PS);

			// =======================================================
			// [추가] PlayerState에서 플레이어 이름 가져오기
			// =======================================================

			if (WinnerSound) PC->ClientPlaySound(WinnerSound);
			
			FString PlayerName = PS->GetPlayerData().PlayerName;
			if (PlayerName.IsEmpty())
			{
				PlayerName = PS->GetPlayerName(); // 데이터가 없으면 스팀/기본 닉네임 사용
			}

			FVector SpawnLoc = FVector::ZeroVector;
			FRotator SpawnRot = FRotator::ZeroRotator;
			bool bValidSpotFound = false;

			if (bIsWinner)
			{
				FPTWLastWinnerInfo WinnerInfo;
				WinnerInfo.WinnerId = PS->GetUniqueId().ToString();

				LastWinnerInfo.Add(WinnerInfo);
				
				if (WinSpots.IsValidIndex(CurrentWinIndex))
				{
					SpawnLoc = WinSpots[CurrentWinIndex]->GetActorLocation();
					SpawnRot = WinSpots[CurrentWinIndex]->GetActorRotation();
					CurrentWinIndex++;
					bValidSpotFound = true;

					if (WinnerEffect)
					{
						UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, WinnerEffect, SpawnLoc, SpawnRot);
					}
				}
			}
			else
			{
				if (LoseSpots.IsValidIndex(CurrentLoseIndex))
				{
					SpawnLoc = LoseSpots[CurrentLoseIndex]->GetActorLocation();
					SpawnRot = LoseSpots[CurrentLoseIndex]->GetActorRotation();
					CurrentLoseIndex++;
					bValidSpotFound = true;
				}
			}

			if (bValidSpotFound)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				APTWResultCharacter* ResultChar = World->SpawnActor<APTWResultCharacter>(ResultCharacterClass, SpawnLoc, SpawnRot, SpawnParams);

				if (ResultChar)
				{
					// [수정] 스폰된 결과 캐릭터에게 승패 여부와 함께 '이름'도 전달!
					ResultChar->InitializeResult(bIsWinner, PlayerName);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[Result] 스폰 포인트가 부족합니다! 맵에 TargetPoint를 더 배치해주세요."));
			}
		}
	}
	
	if (GameState->GetCurrentGamePhase() == EPTWGamePhase::MiniGameResult)
	{
		GetWorld()->GetTimerManager().SetTimer(ResultTimerHandle, this, &UPTWResultSequenceComponent::FinishEndGameSequence, ResultSequenceDuration, false);
	}
	else if (GameState->GetCurrentGamePhase() == EPTWGamePhase::GameResult)
	{
		GetWorld()->GetTimerManager().SetTimer(ResultTimerHandle, this, &UPTWResultSequenceComponent::ReturnToMainMenu, 10.f);
	}
	
	
}


void UPTWResultSequenceComponent::FinishEndGameSequence()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APTWPlayerController* PC = Cast<APTWPlayerController> (It->Get());
		if (!PC) continue;

		PC->ChangeState(NAME_Playing);
		PC->ClientGotoState(NAME_Playing);

		if (PC->PlayerState)
		{
			PC->PlayerState->SetIsSpectator(false);
			PC->PlayerState->SetIsOnlyASpectator(false);
		}

		PC->SetControllerComponent(nullptr);
	}

	IPTWMiniGameModeInterface* GameModeInterface = Cast<IPTWMiniGameModeInterface>(GetOwner());
	if (!GameModeInterface) return;
	
	GameModeInterface->TravelLevel();
}

void UPTWResultSequenceComponent::ReturnToMainMenu()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get()))
		{
			if (!PC->IsLocalController())
			{
				PC->Client_OpenMainMenu();
			}
		}
	}
	// 리슨 서버일 경우 호스트는 따로 구현
	UPTWGameInstance* GI = GetWorld()->GetGameInstance<UPTWGameInstance>();
	if (!GI) return;
	
	GI->LeaveGameSession();
}

bool UPTWResultSequenceComponent::IsWinner(APTWPlayerState* PlayerState)
{
	APTWGameMode* GameMode = Cast<APTWGameMode>(GetOwner());
	if (!GameMode) return false;

	return GameMode->IsWinner(PlayerState);
}
