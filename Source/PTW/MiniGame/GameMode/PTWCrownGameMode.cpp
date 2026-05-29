#include "PTWCrownGameMode.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "GAS/PTWAttributeSet.h"


#define LOCTEXT_NAMESPACE "CrownGameMode"
namespace CrownText
{
	const FText Begin						= LOCTEXT("Begin", "왕관을 빼앗으세요.");
	const FText KingIsDead					= LOCTEXT("KingIsDead", "왕 {0}가 죽었습니다.");
	const FText NewKingCrowned				= LOCTEXT("NewKingCrowned", "{0}이 새로운 왕이 되었습니다!");
	const FText NewKingStoleCrown			= LOCTEXT("NewKingStolenCrown", "{0}이 왕관을 빼앗고 새로운 왕이 되었습니다!");
}
#undef LOCTEXT_NAMESPACE

APTWCrownGameMode::APTWCrownGameMode()
{
	MiniGameRule.TimeRule.bUseTimer = true;
	MiniGameRule.TimeRule.Round = 1;
	MiniGameRule.TimeRule.Timer = 120.0f;
	
	MiniGameRule.TimeRule.bUseCountDown = true;
	MiniGameRule.TimeRule.CountDown = 5.0f;
	
	MiniGameRule.SpawnRule.bUseRespawn = true;
	
	MiniGameRule.CombatRule.bAllowGun = true;
	MiniGameRule.CombatRule.bAllowMelee = true;
	
	MiniGameRule.WinConditionRule.WinType = EPTWWinType::Survival;
	
	MiniGameRule.TeamRule.bUseTeam = false;
	
	// Health 500%, ATK 200%, MovementSpeed -30%
}

void APTWCrownGameMode::RemovePrevKing()
{
	APTWPlayerState* PrevKing = CurrentKing;

	UAbilitySystemComponent* PrevKingASC = Cast<IAbilitySystemInterface>(PrevKing) ? 
		Cast<IAbilitySystemInterface>(PrevKing)->GetAbilitySystemComponent() : nullptr;

	if (IsValid(PrevKingASC) && IsValid(KingBuffGameplayEffect))
	{
		// 이전 왕은 버프 제거
		PrevKingASC->RemoveActiveGameplayEffectBySourceEffect(KingBuffGameplayEffect, PrevKingASC);
	}
	
	const FString DeadKingName = PrevKing->GetPlayerName();
	const FText FinalMessage = FText::Format(CrownText::KingIsDead, FText::FromString(DeadKingName));
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = IsValid(It->Get()) ? Cast<APTWPlayerController>(It->Get()) : nullptr)
		{
			PC->SendMessage(FinalMessage, ENotificationPriority::High, 3.0f);
		}
	}
}

void APTWCrownGameMode::SetPreliminaryKing(APTWPlayerState* NewKing)
{
	PreliminaryKing = NewKing;
	
	UAbilitySystemComponent* NewKingASC = Cast<IAbilitySystemInterface>(NewKing) ? 
		Cast<IAbilitySystemInterface>(NewKing)->GetAbilitySystemComponent() : nullptr;
	
	if (IsValid(NewKingASC) && IsValid(InvincibleGameplayEffect))
	{
		FGameplayEffectContextHandle Context = NewKingASC->MakeEffectContext();
		Context.AddSourceObject(NewKingASC->GetAvatarActor());
		
		FGameplayEffectSpecHandle InvincibleSpecHandle =
			NewKingASC->MakeOutgoingSpec(InvincibleGameplayEffect, 1.0f, Context);
		if (InvincibleSpecHandle.IsValid())
		{
			// 왕관이 머리에 씌워질 때 까지 무적
			NewKingASC->ApplyGameplayEffectSpecToSelf(*InvincibleSpecHandle.Data.Get());
		}
	}
	
	SpawnedCrown->OnAttachToOwnerHead.AddUniqueDynamic(this, &ThisClass::ConfirmPreliminaryKing);
	
	if (APTWPlayerCharacter* Character = NewKing->GetPawn<APTWPlayerCharacter>())
	{
		SpawnedCrown->SetPreliminaryOwner(Character);
		SpawnedCrown->StartMovingToOwner();
	}
}

void APTWCrownGameMode::ConfirmPreliminaryKing()
{
	CurrentKing = PreliminaryKing;
	PreliminaryKing = nullptr;
	
	UAbilitySystemComponent* NewKingASC = Cast<IAbilitySystemInterface>(CurrentKing) ? 
		Cast<IAbilitySystemInterface>(CurrentKing)->GetAbilitySystemComponent() : nullptr;

	if (IsValid(NewKingASC) && IsValid(KingBuffGameplayEffect) && IsValid(InvincibleGameplayEffect))
	{
		NewKingASC->RemoveActiveGameplayEffectBySourceEffect(InvincibleGameplayEffect, NewKingASC);
		
		FGameplayEffectContextHandle Context = NewKingASC->MakeEffectContext();
		Context.AddSourceObject(NewKingASC->GetAvatarActor());
		
		FGameplayEffectSpecHandle KingBuffSpecHandle =
			NewKingASC->MakeOutgoingSpec(KingBuffGameplayEffect, 1.0f, Context);
		if (KingBuffSpecHandle.IsValid())
		{
			// 새로운 왕은 버프 생성
			NewKingASC->ApplyGameplayEffectSpecToSelf(*KingBuffSpecHandle.Data.Get());
		}
	}
	
	// 새로운 왕은 체력회복
	if (UPTWAttributeSet* AttributeSet = Cast<UPTWAttributeSet>(CurrentKing->GetAttributeSet()))
	{
		AttributeSet->SetHealth(AttributeSet->GetMaxHealth());
	}
	
	SpawnedCrown->OnAttachToOwnerHead.RemoveDynamic(this, &ThisClass::ConfirmPreliminaryKing);
	SpawnedCrown->OnDetachFromOwner.AddUniqueDynamic(this, &ThisClass::RemovePrevKing);
	
	const FString NewKingName = CurrentKing->GetPlayerName();
	const FText NewKingMessage = bIsFirstCrown ? CrownText::NewKingCrowned : CrownText::NewKingStoleCrown;
	const FText FinalMessage = FText::Format(CrownText::NewKingCrowned, FText::FromString(NewKingName));
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = IsValid(It->Get()) ? Cast<APTWPlayerController>(It->Get()) : nullptr)
		{
			PC->SendMessage(FinalMessage, ENotificationPriority::High, 5.0f);
		}
	}
	bIsFirstCrown = false;
}

void APTWCrownGameMode::RandomSelectKing()
{
	if (IsValid(PTWGameState))
	{
		int32 MaxAlivePlayers = PTWGameState->AlivePlayers.Num();
		int32 SelectedKingIndex = FMath::RandHelper(MaxAlivePlayers - 1);
		
		auto It = PTWGameState->AlivePlayers.CreateConstIterator();
		for (int32 i = 0; i < SelectedKingIndex; ++i) 
		{
			++It;
		}
		if (APTWPlayerState* PS = Cast<APTWPlayerState>(*It))
		{
			SetPreliminaryKing(PS);
		}
	}
}

void APTWCrownGameMode::CleanupCrownSystem()
{
	if (IsValid(SpawnedCrown))
	{
		SpawnedCrown->OnAttachToOwnerHead.RemoveDynamic(this, &ThisClass::ConfirmPreliminaryKing);
		SpawnedCrown->OnDetachFromOwner.RemoveDynamic(this, &ThisClass::RemovePrevKing);
	}
	
	GetWorldTimerManager().ClearTimer(ScoreTimerHandle);
}

void APTWCrownGameMode::BeginPlay()
{
	Super::BeginPlay();
	
}

void APTWCrownGameMode::StartPlay()
{
	Super::StartPlay();
}

void APTWCrownGameMode::WaitingToStartRound()
{
	Super::WaitingToStartRound();
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = IsValid(It->Get()) ? Cast<APTWPlayerController>(It->Get()) : nullptr)
		{
			PC->SendMessage(CrownText::Begin, ENotificationPriority::Normal, 5.0f);
		}
	}
}

void APTWCrownGameMode::StartRound()
{
	Super::StartRound();
	
	RandomSelectKing();
	
	GetWorldTimerManager().SetTimer(ScoreTimerHandle, this, &ThisClass::ScoreTimer, 1.0f, true);
}

void APTWCrownGameMode::HandlePlayerDeath(AActor* Victim, AActor* AttackerPS)
{
	Super::HandlePlayerDeath(Victim, AttackerPS);

	// TODO: Attacker가 PlayerState가 들어가는 문제가 있음
	APTWPlayerCharacter* VictimCharacter = Cast<APTWPlayerCharacter>(Victim);
	if (!IsValid(VictimCharacter)) return;
	
	APTWPlayerState* VictimPS = VictimCharacter->GetPlayerState<APTWPlayerState>();
	if (!IsValid(VictimPS)) return;
	
	if (VictimPS == CurrentKing)
	{
		if (Victim != AttackerPS)
		{
			// APTWPlayerCharacter* AttackerCharacter = Cast<APTWPlayerCharacter>(AttackerPS);
			if (IsValid(VictimCharacter))
			{
				if (APTWPlayerState* AttackerPTWPS = Cast<APTWPlayerState>(AttackerPS))
				{
					SpawnedCrown->DetachFromOwner();
					SetPreliminaryKing(AttackerPTWPS);
					return;
				}
			}
		}

		RandomSelectKing();
	}
}

void APTWCrownGameMode::EndGame()
{
	Super::EndGame();
	
	CleanupCrownSystem();
}

void APTWCrownGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	
	Super::EndPlay(EndPlayReason);
}

void APTWCrownGameMode::ScoreTimer()
{
	if (!IsValid(CurrentKing)) return;
	
	CurrentKing->AddScore(1);
}
