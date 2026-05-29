#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Server/PTWServerSettings.h"
#include "PTWSteamSessionSubsystem.generated.h"

namespace SteamSessionText
{
	extern const FText ProcessRequestFailed;
	extern const FText SteamAPILoginFailed;
	
	extern const FText CreateGameSessionFailed;
	extern const FText JoinGameSessionFailed;
	extern const FText JoinGameSessionFailed_Parsing;
	extern const FText SearchForGameSessionsFailed;
}

USTRUCT(BlueprintType)
struct FOnlineSessionSearchResultBP
{
	GENERATED_USTRUCT_BODY()
	FOnlineSessionSearchResult OnlineSessionSearchResult;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionSearchComplete, const TArray<FOnlineSessionSearchResultBP>&, SearchResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllSessionSearchFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSteamSessionMessageReceived, const FText&, Message);

/*
 * 스팀 세션을 관리하는 서브 시스템입니다.
 */
UCLASS()
class PTW_API UPTWSteamSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UPTWSteamSessionSubsystem* Get(const UObject* WorldContextObject);
	IOnlineSessionPtr GetSessionInterface() const;
	IOnlineIdentityPtr GetIdentityInterface() const;
	// 현재 Server의 SteamId를 반환
	UFUNCTION(BlueprintCallable, Category = "Session")
	FString GetSteamServerId() const;

	// 리슨&전용서버 전용 레벨이동 함수
	void SteamServerTravel(const FName& MapName) const;
	
	void CreateGameSession(const FPTWServerSettings& ServerSettings);
	void FindGameSession();
	void SearchForGameSessions();
	void JoinGameSession(const FOnlineSessionSearchResultBP& SearchResult, const FString Options = TEXT(""));
	void DestroySession();

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	void HandlePostWorldInitialization(UWorld* World, const UWorld::InitializationValues Ivs);
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UniqueId, const FString& Error);
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result, const FString Options);
	void OnFindSessionsComplete(bool bWasSuccessful);

protected:
	TArray<FOnlineSessionSearchResultBP> BPSearchResults;
	TQueue<TSharedPtr<FOnlineSessionSearch>> SessionSearchQueue;

public:
	FOnSessionSearchComplete OnSessionSearchComplete;
	FOnAllSessionSearchFinished OnAllSessionSearchFinished;
	FOnSteamSessionMessageReceived OnSteamSessionMessageReceived;
	
protected:
	FDelegateHandle PostWorldInitializationDelegateHandle;
	FDelegateHandle LoginCompletedDelegateHandle;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionDelegateHandle;
};
