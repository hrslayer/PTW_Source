#include "PTWGameLiftConnectionComponent.h"
#include "GameLiftServerSDK.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "System/PTWGameLiftServerSubsystem.h"
#include "CoreFramework/Game/GameMode/PTWGameMode.h"


void UPTWGameLiftConnectionComponent::OnRegister()
{
	Super::OnRegister();
	
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UGameFrameworkComponentManager* ComponentManager = GameInstance->GetSubsystem<UGameFrameworkComponentManager>())
			{
				OwnerExtensionHandle = ComponentManager->AddExtensionHandler(
					APTWGameMode::StaticClass(),
					UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleOwnerExtension));
			}
		}
	}
}

void UPTWGameLiftConnectionComponent::OnUnregister()
{
	OwnerExtensionHandle.Reset();
	
	Super::OnUnregister();
}

void UPTWGameLiftConnectionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APTWGameMode* GameMode = Cast<APTWGameMode>(GetOwner()))
	{
		GameMode->OnPreLogin.RemoveDynamic(this, &ThisClass::HandlePreLogin);
		GameMode->OnPostLogin.RemoveDynamic(this, &ThisClass::HandlePostLogin);
		GameMode->OnPreLogout.RemoveDynamic(this, &ThisClass::HandlePreLogout);
		GameMode->OnPostLogout.RemoveDynamic(this, &ThisClass::HandlePostLogout);
	}
	
	Super::EndPlay(EndPlayReason);
}

void UPTWGameLiftConnectionComponent::HandleOwnerExtension(AActor* Receiver, FName EventName)
{
	if (EventName.IsEqual(UGameFrameworkComponentManager::NAME_GameActorReady))
	{
		if (APTWGameMode* GameMode = GetOwner<APTWGameMode>())
		{
			GameMode->OnPreLogin.AddUniqueDynamic(this, &ThisClass::HandlePreLogin);
			GameMode->OnPostLogin.AddUniqueDynamic(this, &ThisClass::HandlePostLogin);
			GameMode->OnPreLogout.AddUniqueDynamic(this, &ThisClass::HandlePreLogout);
			GameMode->OnPostLogout.AddUniqueDynamic(this, &ThisClass::HandlePostLogout);
		}
	}
}

void UPTWGameLiftConnectionComponent::HandlePreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	FString PlayerSessionId = UGameplayStatics::ParseOption(Options, TEXT("PlayerSessionId"));
	FString PlayerUniqueId = UniqueId.IsValid() ? UniqueId.ToString() : TEXT("");
	if (!PlayerSessionId.IsEmpty() && !PlayerUniqueId.IsEmpty())
	{
		if (UPTWGameLiftServerSubsystem* GameLiftServerSubsystem = UPTWGameLiftServerSubsystem::Get(this))
		{
			bool bIsSuccess = GameLiftServerSubsystem->AcceptPlayerSession(PlayerUniqueId, PlayerSessionId);
			if (!bIsSuccess)
			{
				ErrorMessage = TEXT("Invalid GameLift Player Session");
			}
		}
	}
	else
	{
		ErrorMessage = TEXT("Missing PlayerSessionId or UniqueId");
	}
}

void UPTWGameLiftConnectionComponent::HandlePostLogin(APlayerController* NewPlayerController)
{
	if (UPTWGameLiftServerSubsystem* GameLiftServerSubsystem = UPTWGameLiftServerSubsystem::Get(this))
	{
		GameLiftServerSubsystem->UpdatePlayerCount(TEXT("Join"));
	}
}

void UPTWGameLiftConnectionComponent::HandlePreLogout(AController* Exiting)
{
	FString UniqueId;
	if (APlayerState* PlayerState = Exiting->GetPlayerState<APlayerState>())
	{
		UniqueId = PlayerState->GetUniqueId().IsValid() ? PlayerState->GetUniqueId().ToString() : TEXT("");
	}
	
	UPTWGameLiftServerSubsystem* GameLiftServerSubsystem = UPTWGameLiftServerSubsystem::Get(this);
	if (IsValid(GameLiftServerSubsystem) && !UniqueId.IsEmpty())
	{
		// 로그아웃한 PlayerSession 제거
		GameLiftServerSubsystem->RemovePlayerSession(UniqueId);
	}
}

void UPTWGameLiftConnectionComponent::HandlePostLogout(AController* Exiting)
{
	UPTWGameLiftServerSubsystem* GameLiftServerSubsystem = UPTWGameLiftServerSubsystem::Get(this);
	AGameModeBase* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr;
	if (IsValid(GameLiftServerSubsystem) && IsValid(GameMode))
	{
		int32 CurrentPlayers = GameMode->GetNumPlayers();
		
		if (CurrentPlayers > 0)
		{
			GameLiftServerSubsystem->UpdatePlayerCount(TEXT("Leave"));
		}
		else if (CurrentPlayers <= 0)
		{
			TerminateGameSession();
		}
	}
}

void UPTWGameLiftConnectionComponent::TerminateGameSession()
{
	if (UPTWGameLiftServerSubsystem* GameLiftServerSubsystem = UPTWGameLiftServerSubsystem::Get(this))
	{
		GameLiftServerSubsystem->TerminateGameSession();
	}
}
