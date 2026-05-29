#include "PTWVoiceChatSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "CoreFramework/PTWGameUserSettings.h"
#include "Interfaces/VoiceInterface.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"

UPTWVoiceChatSubsystem* UPTWVoiceChatSubsystem::Get(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull)
		: nullptr)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
			{
				return LocalPlayer->GetSubsystem<UPTWVoiceChatSubsystem>();
			}
		}
	}
	return nullptr;
}

IOnlineVoicePtr UPTWVoiceChatSubsystem::GetVoiceInterface() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(World))
		{
			if (IOnlineVoicePtr VoiceInterface = OnlineSubsystem->GetVoiceInterface())
			{
				return VoiceInterface;
			}
		}
	}
	return nullptr;
}

float UPTWVoiceChatSubsystem::GetPlayerVoiceVolume(const FString& PlayerID)
{
	if (PlayerVoiceInfoList.Contains(PlayerID))
	{
		return PlayerVoiceInfoList[PlayerID].Volume;
	}
	else
	{
		return 1.0f;
	}
}

void UPTWVoiceChatSubsystem::SetPlayerVoiceVolume(const FString& PlayerID, float NewVolume)
{
	if (PlayerVoiceInfoList.Contains(PlayerID))
	{
		PlayerVoiceInfoList[PlayerID].Volume = NewVolume;
	}
	
	if (UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings()))
	{
		if (Settings->PlayerVolumes.Contains(PlayerID))
		{
			Settings->PlayerVolumes[PlayerID] = NewVolume;
		}
	}
}

void UPTWVoiceChatSubsystem::StartVoiceChat(bool bFromInput)
{
	if (!bVoiceEnabled) return;
	if (bFromInput && !bIsPushToTalk) return;

	IOnlineVoicePtr VoiceInterface = GetVoiceInterface();
	if (!VoiceInterface.IsValid()) return;

	const UPTWGameInstance* GI = GetWorld()->GetGameInstance<UPTWGameInstance>();
	if (!IsValid(GI)) return;
	
	if (!GI->LocalUniqueId.IsEmpty() && bIsPushToTalk)
	{
		bIsActive = true;
		HandlePlayerActiveStateChanged(GI->LocalUniqueId, EPTWVoiceState::Requesting);
	}
	VoiceInterface->StartNetworkedVoice(0);
}

void UPTWVoiceChatSubsystem::StopVoiceChat(bool bFromInput)
{
	if (!bVoiceEnabled) return;
	if (bFromInput && !bIsPushToTalk) return;

	IOnlineVoicePtr VoiceInterface = GetVoiceInterface();
	if (!VoiceInterface.IsValid()) return;

	const UPTWGameInstance* GI = GetWorld()->GetGameInstance<UPTWGameInstance>();
	if (!IsValid(GI)) return;
	
	if (!GI->LocalUniqueId.IsEmpty())
	{
		HandlePlayerActiveStateChanged(GI->LocalUniqueId, EPTWVoiceState::Idle);
		bIsActive = false;
	}
	VoiceInterface->StopNetworkedVoice(0);
}

void UPTWVoiceChatSubsystem::SetVoiceInputMode(bool bNewIsPushToTalk)
{
	StopVoiceChat(false);
	bIsPushToTalk = bNewIsPushToTalk;
	if (!bIsPushToTalk)
	{
		StartVoiceChat(false);
	}
}

void UPTWVoiceChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings()))
	{
		bIsPushToTalk = Settings->bIsPushToTalk;
	}
	
	FWorldDelegates::OnWorldCleanup.AddUObject(this, &ThisClass::HandleWorldCleanup);
	OnVoiceChatWidgetReady.AddUniqueDynamic(this, &ThisClass::HandleVoiceChatWidgetReady);
	
	if (IOnlineVoicePtr VoiceInterface = GetVoiceInterface())
	{
		OnPlayerTalkingStateChanged = VoiceInterface->AddOnPlayerTalkingStateChangedDelegate_Handle(
			FOnPlayerTalkingStateChangedDelegate::CreateUObject(this, &ThisClass::HandlePlayerTalkingStateChanged));
	}
	else
	{
		PostWorldInitializationDelegateHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ThisClass::HandlePostWorldInitialization);
	}
	
	if (UPTWGameInstance* GI = GetWorld()->GetGameInstance<UPTWGameInstance>())
	{
		GI->OnSessionPlayerConnected.AddUniqueDynamic(this, &ThisClass::HandlePlayerConnected);
		GI->OnSessionPlayerDisconnected.AddUniqueDynamic(this, &ThisClass::HandlePlayerDisconnected);
	}
}

void UPTWVoiceChatSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	OnVoiceChatWidgetReady.RemoveDynamic(this, &ThisClass::HandleVoiceChatWidgetReady);
	
	if (IOnlineVoicePtr VoiceInterface = GetVoiceInterface())
	{
		VoiceInterface->ClearOnPlayerTalkingStateChangedDelegates(this);
	}
	if (UPTWGameInstance* GI = GetWorld()->GetGameInstance<UPTWGameInstance>())
	{
		GI->OnSessionPlayerConnected.RemoveDynamic(this, &ThisClass::HandlePlayerConnected);
		GI->OnSessionPlayerDisconnected.RemoveDynamic(this, &ThisClass::HandlePlayerDisconnected);
	}
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	PlayerVoiceInfoList.Empty();
	
	Super::Deinitialize();
}

void UPTWVoiceChatSubsystem::HandlePostWorldInitialization(UWorld* World, const UWorld::InitializationValues Ivs)
{
	if (!IsValid(World) || !World->IsGameWorld()) return;
	
	if (PostWorldInitializationDelegateHandle.IsValid())
	{
		FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitializationDelegateHandle);
	}
	
	if (IOnlineVoicePtr VoiceInterface = GetVoiceInterface())
	{
		OnPlayerTalkingStateChanged = VoiceInterface->AddOnPlayerTalkingStateChangedDelegate_Handle(
			FOnPlayerTalkingStateChangedDelegate::CreateUObject(this, &ThisClass::HandlePlayerTalkingStateChanged));
	}
}

void UPTWVoiceChatSubsystem::HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	if (World != GetWorld()) return;
	
	bVoiceEnabled = false;
	if (IOnlineVoicePtr VoiceInterface = GetVoiceInterface())
	{
		VoiceInterface->StopNetworkedVoice(0);
		VoiceInterface->RemoveAllRemoteTalkers();
	}
	
	for (TObjectIterator<UActorComponent> It; It; ++It)
	{
		if (It->GetName().Contains(TEXT("VoipListener")))
		{
			if (It->IsRegistered())
			{
				It->UnregisterComponent();
			}
			It->DestroyComponent();
		}
	}
}

void UPTWVoiceChatSubsystem::HandlePlayerConnected(const FString& UniqueId)
{
	FPTWPlayerVoiceInfo PlayerVoiceInfo;
	if (UPTWGameInstance* GI = GetWorld()->GetGameInstance<UPTWGameInstance>())
	{
		if (FString* PlayerName = GI->SessionPlayerNames.Find(UniqueId))
		{
			PlayerVoiceInfo.PlayerName = *PlayerName;
		}
	}
	PlayerVoiceInfoList.Add(UniqueId, PlayerVoiceInfo);
	if (UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings()))
	{
		if (auto PlayerVolume = Settings->PlayerVolumes.Find(UniqueId))
		{
			PlayerVoiceInfoList[UniqueId].Volume = *PlayerVolume;
		}
		else
		{
			Settings->PlayerVolumes.Add(UniqueId, PlayerVoiceInfoList[UniqueId].Volume);
		}
	}
	OnVoiceChatConnected.Broadcast(UniqueId);
}

void UPTWVoiceChatSubsystem::HandlePlayerDisconnected(const FString& UniqueId)
{
	PlayerVoiceInfoList.Remove(UniqueId);
	OnVoiceChatDisconnected.Broadcast(UniqueId);
}

void UPTWVoiceChatSubsystem::HandlePlayerTalkingStateChanged(TSharedRef<const FUniqueNetId> TalkerId, bool bIsTalking)
{
	FString UniqueId = TalkerId->ToString();
	
	EPTWVoiceState VoiceState = bIsTalking ? EPTWVoiceState::Talking : EPTWVoiceState::Idle;
	if (UPTWGameInstance* GI = GetWorld()->GetGameInstance<UPTWGameInstance>())
	{
		const FString& LocalUniqueId = GI->LocalUniqueId;
	
		if (!LocalUniqueId.IsEmpty() && LocalUniqueId == UniqueId && bIsPushToTalk && VoiceState == EPTWVoiceState::Idle)
		{
			VoiceState = bIsActive ? EPTWVoiceState::Requesting : EPTWVoiceState::Idle;
		}
	}

	HandlePlayerActiveStateChanged(UniqueId, VoiceState);
}

void UPTWVoiceChatSubsystem::HandlePlayerActiveStateChanged(const FString& UniqueId, EPTWVoiceState VoiceState)
{
	if (PlayerVoiceInfoList.Contains(UniqueId))
	{
		PlayerVoiceInfoList[UniqueId].VoiceState = VoiceState;
		OnPlayerVoiceInfoUpdated.Broadcast(UniqueId);
	}
}

void UPTWVoiceChatSubsystem::HandleVoiceChatWidgetReady()
{
	bVoiceEnabled = true;
	if (!bIsPushToTalk)
	{
		StartVoiceChat(false);
	}
}
