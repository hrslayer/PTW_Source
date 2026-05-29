#pragma once
#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "System/Server/PTWServerSettings.h"
#include "PTWGameLiftServerSubsystem.generated.h"

class FGameLiftServerSDKModule;

// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameLiftServerResponseReceived, const bool, bWasSuccessful);

/**
 * 서버 전용 로직을 관리하는 게임리프트 서브 시스템입니다.
 */
UCLASS()
class PTWDEDICATEDSERVER_API UPTWGameLiftServerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UPTWGameLiftServerSubsystem();
	static UPTWGameLiftServerSubsystem* Get(const UObject* WorldContextObject);
	
	FORCEINLINE FGameLiftServerSDKModule* GetGameLiftSdkModule() const { return GameLiftSdkModule; };
	IOnlineSessionPtr GetSessionInterface() const;
	void SetGameLiftSdkModule(FGameLiftServerSDKModule* InGameLiftSdkModule) { GameLiftSdkModule = InGameLiftSdkModule; };
	
	void ActivateGameSession();
	void HeartbeatGameSession();
	void TerminateGameSession();
	void UpdatePlayerCount(FString Action);
	bool AcceptPlayerSession(const FString& UniqueId, const FString& PlayerSessionId);
	void RemovePlayerSession(const FString& UniqueId);
	void StopAutoBackfill();

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	void Cleanup();
	
	void ActivateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void HeartbeatGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void TerminateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void UpdatePlayerCount_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "NetWork")
	TMap<FString, FString> PlayerSessionIds;
	
	FString CurrentBackfillTicketId;
	FMatchBackfillData MatchBackfillData;
	
protected:
	FGameLiftServerSDKModule* GameLiftSdkModule;
	
public:
	FTimerHandle HeartbeatTimerHandle;
	
private:
	FDelegateHandle UpdateSessionCompleteDelegateHandle;

};
