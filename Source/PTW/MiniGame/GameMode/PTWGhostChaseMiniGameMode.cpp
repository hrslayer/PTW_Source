// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/GameMode/PTWGhostChaseMiniGameMode.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/Character/Component/PTWUIControllerComponent.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"

#include "System/PTWItemSpawnManager.h"
#include "Inventory/PTWInventoryComponent.h"
#include "MiniGame/ControllerComponent/GhostChase/PTWGhostChaseControllerComponent.h"

#include "GAS/PTWAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MiniGame/Data/PTWResultStateData.h"

#define LOCTEXT_NAMESPACE "PTWGhostChaseMiniGameMode"

APTWGhostChaseMiniGameMode::APTWGhostChaseMiniGameMode()
{
	// 규칙 설정
	MiniGameRule.WinConditionRule.WinType = EPTWWinType::Survival;

	MiniGameRule.TimeRule.bUseCountDown = true;
	MiniGameRule.TimeRule.CountDown = 10.f; // 10초 대기

	MiniGameRule.TimeRule.bUseTimer = true;
	MiniGameRule.TimeRule.Timer = 180.f;    // 3분 게임
}

void APTWGhostChaseMiniGameMode::HandlePlayerDeath(AActor* DeadActor, AActor* KillActor)
{
	Super::HandlePlayerDeath(DeadActor, KillActor);

	AController* DeadController = nullptr;

	if (APawn* DeadPawn = Cast<APawn>(DeadActor))
	{
		DeadController = DeadPawn->GetController();
	}

	if (DeadController)
	{
		OnPlayerEliminated(DeadController);
	}
}

void APTWGhostChaseMiniGameMode::OnPlayerEliminated(AController* EliminatedController)
{
	if (!EliminatedController) return;

	if (auto* GCComp = EliminatedController->FindComponentByClass<UPTWGhostChaseControllerComponent>())
	{
		GCComp->SetTarget(nullptr);
	}

	if (ActiveChasers.Num() == 0) return;

	int32 ElimIndex = ActiveChasers.Find(EliminatedController);
	if (ElimIndex == INDEX_NONE) return;

	if (ActiveChasers.Num() <= 1)
	{
		ActiveChasers.RemoveAt(ElimIndex);
		TargetMap.Remove(EliminatedController);
		EndGame();
		return;
	}

	int32 NumChasers = ActiveChasers.Num();

	// 누구의 타겟이었는지 찾기 (내 앞 사람)
	int32 ChaserIndex = (ElimIndex - 1 + NumChasers) % NumChasers;
	AController* ChaserOfDeadPlayer = ActiveChasers[ChaserIndex];

	// 나의 다음 타겟 찾기 (내 뒷 사람)
	int32 NewTargetIndex = (ElimIndex + 1) % NumChasers;
	AController* NewTargetForChaser = ActiveChasers[NewTargetIndex];

	// 배열에서 제거
	ActiveChasers.RemoveAt(ElimIndex);
	TargetMap.Remove(EliminatedController);

	// 타겟 재설정: 죽은 자를 쫓던 사람(Chaser)이 죽은 자의 타겟(NewTarget)을 쫓게 함
	if (ChaserOfDeadPlayer && NewTargetForChaser && ChaserOfDeadPlayer != NewTargetForChaser)
	{
		TargetMap.Add(ChaserOfDeadPlayer, NewTargetForChaser);
		UpdatePlayerTargetUI(ChaserOfDeadPlayer, NewTargetForChaser);

		AddStatValue(ChaserOfDeadPlayer, EPTWResultStatName::GhostChase_ChasedCount, 1); // 새로운 타겟을 추적함
		AddStatValue(NewTargetForChaser, EPTWResultStatName::GhostChase_ChaserCount, 1); // 새로운 사람에게 추적당함
	}

	TargetMap.Remove(EliminatedController);

	// 최후의 1인 체크
	if (ActiveChasers.Num() == 1)
	{
		EndGame();
	}
}

bool APTWGhostChaseMiniGameMode::IsValidChaseTarget(AController* Chaser, AController* Target) const
{
	if (!Chaser || !Target)
	{
		return false;
	}

	const AController* const* FoundTarget = TargetMap.Find(Chaser);

	if (!FoundTarget)
	{
		return false;
	}

	return (*FoundTarget == Target);
}

void APTWGhostChaseMiniGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void APTWGhostChaseMiniGameMode::EndGame()
{
	if (bIsGameEnded) return;

	if (!PTWGameState) return;

	// 순위 업데이트 수행
	// WinType이 Survival이므로 [생존자 -> 마지막 사망자 -> ... -> 최초 사망자] 순으로 정렬됨
	PTWGameState->UpdateRanking(MiniGameRule);

	// 내부에서 PTWGameState->ApplyMiniGameRankScore(MiniGameRule)가 실행
	Super::EndGame();
}

void APTWGhostChaseMiniGameMode::StartGame()
{
	Super::StartGame();

	BindDamageEvent();
}

void APTWGhostChaseMiniGameMode::WaitingToStartRound()
{
	Super::WaitingToStartRound();

	// 필요 시 대기 시간 동안 플레이어 움직임을 제한하는 등의 로직 추가
}

void APTWGhostChaseMiniGameMode::StartRound()
{
	Super::StartRound();

	// 타겟 체인 구성
	SetupTargetChain();

	// 기본무기 지급
	GiveBaseWeaponToAll();

	// 닉네임 구분시작
	StartNameDistinguish();

	// 투명화 적용
	//ApplyInvisibilityToAll();
}

void APTWGhostChaseMiniGameMode::StartCountDown()
{
	NotificateMessage();
	Super::StartCountDown();
}

void APTWGhostChaseMiniGameMode::SetupTargetChain()
{
	ActiveChasers.Empty();
	TargetMap.Empty();

	// 월드의 모든 플레이어 컨트롤러 수집
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (IsValid(PC) && IsValid(PC->GetPawn()))
		{
			ActiveChasers.Add(PC);
		}
	}

	if (ActiveChasers.Num() < 2) return;

	// 랜덤하게 섞기
	const int32 NumActivePlayers = ActiveChasers.Num();
	for (int32 i = 0; i < NumActivePlayers; i++)
	{
		int32 DestIndex = FMath::RandRange(i, NumActivePlayers - 1);
		ActiveChasers.Swap(i, DestIndex);
	}

	// 순환 구조 연결 (i -> i+1)
	for (int32 i = 0; i < NumActivePlayers; i++)
	{
		AController* Chaser = ActiveChasers[i];
		AController* Target = ActiveChasers[(i + 1) % NumActivePlayers];

		TargetMap.Add(Chaser, Target);
		UpdatePlayerTargetUI(Chaser, Target);

		AddStatValue(Chaser, EPTWResultStatName::GhostChase_ChasedCount, 1); // 내가 추적을 시작함
		AddStatValue(Target, EPTWResultStatName::GhostChase_ChaserCount, 1); // 나를 추적하기 시작함
	}
}

void APTWGhostChaseMiniGameMode::ApplyInvisibilityToAll()
{
	
}

void APTWGhostChaseMiniGameMode::UpdatePlayerTargetUI(AController* Chaser, AController* NewTarget)
{
	if (!IsValid(Chaser) || !IsValid(NewTarget)) return;

	// Target의 Pawn
	APawn* TargetPawn = NewTarget->GetPawn();

	if (IsValid(Chaser))
	{
		if (auto* GCComp = Chaser->FindComponentByClass<UPTWGhostChaseControllerComponent>())
		{
			GCComp->SetTarget(TargetPawn);
		}
	}
}

void APTWGhostChaseMiniGameMode::GiveBaseWeaponToAll()
{
	UPTWItemSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>();
	if (!SpawnManager) return;

	// 현재 게임에 참여 중인 모든 추격자(플레이어) 순회
	for (AController* Controller : ActiveChasers)
	{
		if (!Controller) continue;

		APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(Controller->GetPawn());
		APTWPlayerState* PS = Controller->GetPlayerState<APTWPlayerState>();

		if (PC && PS)
		{
			// 아이템 스폰
			SpawnManager->SpawnSingleItem(PS, GhostWeaponDef);

			// 무기 지급
			if (UPTWInventoryComponent* Inven = PC->GetInventoryComponent())
			{
				Inven->EquipWeapon(0);
			}
		}
	}
}

void APTWGhostChaseMiniGameMode::StartNameDistinguish()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get()))
		{
			// PC 내부의 UI 컴포넌트를 찾아 초기화 함수 호출
			if (auto* UIComp = PC->FindComponentByClass<UPTWUIControllerComponent>())
			{
				UIComp->Client_FindGhostChaseComponent();
			}
		}
	}
}

void APTWGhostChaseMiniGameMode::NotificateMessage()
{
	for (APlayerState* AS : PTWGameState->PlayerArray)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(AS->GetPlayerController()))
		{
			FText BeginSendMessage1 = LOCTEXT("GhostChaseBeginMsg01", "자신의 목표만 공격할 수 있습니다. 목표를 차례대로 제거하고 최후의 생존자가 되세요.");
			PC->SendMessage(BeginSendMessage1, ENotificationPriority::Normal, 5);

			FText BeginSendMessage2 = LOCTEXT("GhostChaseBeginMsg02", "게임이 시작되면 목표의 시점이 왼쪽 상단에 표시됩니다.");
			PC->SendMessage(BeginSendMessage2, ENotificationPriority::Normal, 5);
		}
	}
}

void APTWGhostChaseMiniGameMode::AddStatValue(AController* Controller, EPTWResultStatName StatName, int32 Amount)
{
	if (!Controller) return;

	if (APlayerState* PS = Controller->GetPlayerState<APlayerState>())
	{
		FString UniqueId = PS->GetUniqueId().ToString();
		AddResultDataValue(UniqueId, StatName, Amount);
	}
}

void APTWGhostChaseMiniGameMode::BindDamageEvent()
{
	if (!PTWGameState) return;

	for (APlayerState* PlayerState : PTWGameState->PlayerArray)
	{
		APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState);
		if (!PTWPlayerState) continue;

		UAbilitySystemComponent* ASC = PTWPlayerState->GetAbilitySystemComponent();
		if (!ASC) continue;

		if (const UPTWAttributeSet* AttributeSet = ASC->GetSet<UPTWAttributeSet>())
		{
			UPTWAttributeSet* MutableSet = const_cast<UPTWAttributeSet*>(AttributeSet);

			MutableSet->OnDamageApplied.AddUObject(this, &APTWGhostChaseMiniGameMode::OnTrackedDamageApplied);
		}
	}
}

void APTWGhostChaseMiniGameMode::OnTrackedDamageApplied(UAbilitySystemComponent* TargetASC, UAbilitySystemComponent* SourceASC, float Damage)
{
	if (!SourceASC || SourceASC == TargetASC) return;

	APlayerState* SourcePlayerState = Cast<APlayerState>(SourceASC->GetOwner());
	if (!SourcePlayerState) return;

	FString UniqueId = SourcePlayerState->GetUniqueId().ToString();
	AddResultDataValue(UniqueId, EPTWResultStatName::GhostChase_Damage, Damage);
}

#undef LOCTEXT_NAMESPACE
