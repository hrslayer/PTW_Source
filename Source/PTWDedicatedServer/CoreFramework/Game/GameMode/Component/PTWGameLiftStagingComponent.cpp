#include "PTWGameLiftStagingComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "CoreFramework/Staging/PTWStagingGameMode.h"
#include "Debug/PTWLogCategorys.h"
#include "System/PTWGameLiftServerSubsystem.h"


void UPTWGameLiftStagingComponent::OnRegister()
{
	Super::OnRegister();
	
	UWorld* World = GetWorld();
	if (!IsValid(World)) return;
	
	UGameInstance* GI = World->GetGameInstance();
	if (!IsValid(GI)) return;
	
	if (UGameFrameworkComponentManager* ComponentManager = GI->GetSubsystem<UGameFrameworkComponentManager>())
	{
		OwnerExtensionHandle = ComponentManager->AddExtensionHandler(
			APTWStagingGameMode::StaticClass(),
			UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleOwnerExtension));
	}
}

void UPTWGameLiftStagingComponent::OnUnregister()
{
	OwnerExtensionHandle.Reset();
	
	Super::OnUnregister();
}

void UPTWGameLiftStagingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APTWStagingGameMode* GameMode = GetOwner<APTWStagingGameMode>())
	{
		GameMode->OnAutoBackfillTimeoutRemainTime.RemoveDynamic(this, &ThisClass::HandleAutoBackfillTimeout);
		GameMode->OnPostLogin.RemoveDynamic(this, &ThisClass::HandlePostLogin);

		if (APTWGameState* GS = GameMode->GetGameState<APTWGameState>())
		{
			GS->OnTimerFinished.RemoveDynamic(this, &ThisClass::HandleEndTimer);
		}
	}
	
	Super::EndPlay(EndPlayReason);
}

void UPTWGameLiftStagingComponent::HandleOwnerExtension(AActor* Receiver, FName EventName)
{
	if (EventName.IsEqual(UGameFrameworkComponentManager::NAME_GameActorReady))
	{
		APTWStagingGameMode* GameMode = GetOwner<APTWStagingGameMode>();
		if (!IsValid(GameMode)) return;
		
		GameMode->OnAutoBackfillTimeoutRemainTime.AddUniqueDynamic(this, &ThisClass::HandleAutoBackfillTimeout);
		GameMode->OnPostLogin.AddUniqueDynamic(this, &ThisClass::HandlePostLogin);
		
		if (UPTWGameLiftServerSubsystem* GameLiftServerSubsystem = UPTWGameLiftServerSubsystem::Get(this))
		{
			GameLiftServerSubsystem->ActivateGameSession();
		}
		
		if (APTWGameState* GS = GameMode->GetGameState<APTWGameState>())
		{
			GS->OnTimerFinished.RemoveDynamic(GameMode, &APTWStagingGameMode::EndTimer);
			GS->OnTimerFinished.AddUniqueDynamic(this, &ThisClass::HandleEndTimer);
		}
		
		if (FTimerManager* TimerManager = GetWorld() ? &GetWorld()->GetTimerManager() : nullptr)
		{
			TimerManager->SetTimer(
				AbandonedSCheckTimerHandle, 
				this, 
				&ThisClass::TerminateGameSession, 
				30.f, 
				false);
		}
	}
}

void UPTWGameLiftStagingComponent::HandleAutoBackfillTimeout(int32 RemainTime)
{
	if (RemainTime < 10)
	{
		APTWStagingGameMode* GameMode = GetOwner<APTWStagingGameMode>();
		if (!IsValid(GameMode)) return;
		
		if (!GameMode->OnAutoBackfillTimeoutRemainTime.IsAlreadyBound(this, &ThisClass::HandleAutoBackfillTimeout))
		{
			return;
		}
		GameMode->OnAutoBackfillTimeoutRemainTime.RemoveDynamic(this, &ThisClass::HandleAutoBackfillTimeout);
		
		if (UPTWGameLiftServerSubsystem* GameLiftServerSubsystem = UPTWGameLiftServerSubsystem::Get(this))
		{
			GameLiftServerSubsystem->StopAutoBackfill();
		}
	}
}

void UPTWGameLiftStagingComponent::HandleEndTimer()
{
	UWorld* World = GetWorld();
	if (!IsValid(World)) return;
	
	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
	if (!IsValid(GS)) return;
	
	if (!GS->OnTimerFinished.IsAlreadyBound(this, &ThisClass::HandleEndTimer))
	{
		return;
	}
	GS->OnTimerFinished.RemoveDynamic(this, &ThisClass::HandleEndTimer);
	
	APTWStagingGameMode* GameMode = Cast<APTWStagingGameMode>(GetOwner());
	if (!IsValid(GameMode)) return;
	
	int32 CurrentPlayers = GameMode->GetNumPlayers();
	int32 MinPlayerCount = 1;
	
	if (UPTWGameInstance* GI = World->GetGameInstance<UPTWGameInstance>())
	{
		const FPTWServerSettings& ServerSettings = GI->ServerSettings;
		if (ServerSettings.IsValid())
		{
			MinPlayerCount = ServerSettings.MinPlayerCount;
		}
	}
	
	// 인원수 불만족
	if (CurrentPlayers < MinPlayerCount)
	{
		FString SystemMessage = TEXT("요구 플레이어 수가 부족합니다. 서버를 종료합니다.");
		GS->Multicast_SystemMessage(SystemMessage);

		UE_LOG(Log_Server, Warning, TEXT("요구 플레이어 수를 만족하지 못해 서버를 종료합니다. [인원수: %d]"), CurrentPlayers);
		
		World->GetTimerManager().SetTimer(
			TerminationTimerHandle, 
			this,
			&ThisClass::TerminateGameSession,
			10.0f,
			false
		);
	}
	else
	{
		UE_LOG(Log_Server, Warning, TEXT("인원 수 만족! 레벨 이동..%d/%d"), CurrentPlayers, MinPlayerCount);
		// 인원수 만족
		GameMode->EndTimer();
	}
}

void UPTWGameLiftStagingComponent::HandlePostLogin(APlayerController* NewPlayerController)
{
	if (FTimerManager* TimerManager = GetWorld() ? &GetWorld()->GetTimerManager() : nullptr)
	{
		if (AbandonedSCheckTimerHandle.IsValid())
		{
			TimerManager->ClearTimer(AbandonedSCheckTimerHandle);
			AbandonedSCheckTimerHandle.Invalidate();
		}
	}
}

void UPTWGameLiftStagingComponent::TerminateGameSession()
{
	if (FTimerManager* TimerManager = GetWorld() ? &GetWorld()->GetTimerManager() : nullptr)
	{
		if (AbandonedSCheckTimerHandle.IsValid())
		{
			TimerManager->ClearTimer(AbandonedSCheckTimerHandle);
			AbandonedSCheckTimerHandle.Invalidate();
		}
		if (TerminationTimerHandle.IsValid())
		{
			TimerManager->ClearTimer(TerminationTimerHandle);
			TerminationTimerHandle.Invalidate();
		}
	}
	if (UPTWGameLiftServerSubsystem* GameLiftServerSubsystem = UPTWGameLiftServerSubsystem::Get(this))
	{
		GameLiftServerSubsystem->TerminateGameSession();
	}
}
