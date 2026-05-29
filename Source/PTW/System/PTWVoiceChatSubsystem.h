#pragma once
#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PTWVoiceChatSubsystem.generated.h"

class FUniqueNetId;

UENUM(BlueprintType)
enum class EPTWVoiceState : uint8
{
	None UMETA(DisplayName="None"),
	
	// VoiceChat 입력대기 상태
	Idle UMETA(DisplayName="Idle"),
	
	// VoiceChat 입력승인대기 상태 (활성화)
	Requesting UMETA(DisplayName="Requesting"),
	
	// VoiceChat 입력중 상태
	Talking UMETA(DisplayName="Talking"),
	
	// VoiceChat 음소거 상태
	Muted UMETA(DisplayName="Muted"),
	
	End UMETA(DisplayName="End")
};

USTRUCT(BlueprintType)
struct FPTWPlayerVoiceInfo
{
	GENERATED_BODY()
	
	UPROPERTY()
	FString PlayerName = FString("");
	
	UPROPERTY(BlueprintReadWrite)
	float Volume = 1.0f;
	
	UPROPERTY(BlueprintReadOnly)
	EPTWVoiceState VoiceState;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceChatConnectionSignature, const FString&, UniqueId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllVoiceChatDisconnectedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerVoiceInfoUpdated, const FString&, UniqueId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVoiceChatWidgetReadySignature);


/**
 * Steam VoiceChat을 관리하는 서브 시스템입니다.
 */
UCLASS()
class PTW_API UPTWVoiceChatSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	static UPTWVoiceChatSubsystem* Get(const UObject* WorldContextObject);
	IOnlineVoicePtr GetVoiceInterface() const;
	float GetPlayerVoiceVolume(const FString& PlayerID);
	void SetPlayerVoiceVolume(const FString& PlayerID, float NewVolume);
	
	void StartVoiceChat(bool bFromInput);
	void StopVoiceChat(bool bFromInput);
	
	void SetVoiceInputMode(bool bNewIsPushToTalk);
	void SetVoiceEnabled(bool bNewVoiceEnabled) { bVoiceEnabled = bNewVoiceEnabled; };
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void HandlePostWorldInitialization(UWorld* World, const UWorld::InitializationValues Ivs);
	void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);
	
	UFUNCTION()
	void HandlePlayerConnected(const FString& UniqueId);
	UFUNCTION()
	void HandlePlayerDisconnected(const FString& UniqueId);
	
	void HandlePlayerTalkingStateChanged(TSharedRef<const FUniqueNetId> TalkerId, bool bIsTalking);
	void HandlePlayerActiveStateChanged(const FString& UniqueId, EPTWVoiceState VoiceState);
	
	UFUNCTION()
	void HandleVoiceChatWidgetReady();
	
public:
	UPROPERTY(BlueprintReadWrite, Category="VoiceChat")
	TMap<FString, FPTWPlayerVoiceInfo> PlayerVoiceInfoList;
	
	FPTWPlayerVoiceInfo* LocalPlayerVoiceInfo;

	// 현재 보이스챗을 활성화 가능한지 여부
	bool bVoiceEnabled = false;
	
	// 현재 음성입력 모드가 PushToTalk 여부
	bool bIsPushToTalk = true;
	
	// 현재 로컬플레이어가 말하고 있는지 여부
	bool bIsActive = false;
	
public:
	FOnPlayerVoiceInfoUpdated OnPlayerVoiceInfoUpdated;
	FOnVoiceChatConnectionSignature OnVoiceChatConnected;
	FOnVoiceChatConnectionSignature OnVoiceChatDisconnected;
	FOnVoiceChatWidgetReadySignature OnVoiceChatWidgetReady;
	
protected:
	FDelegateHandle PostWorldInitializationDelegateHandle;
	FDelegateHandle OnPlayerTalkingStateChanged;
};
