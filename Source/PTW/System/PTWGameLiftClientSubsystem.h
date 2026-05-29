#pragma once
#include "CoreMinimal.h"
#include "type_traits"
#include "Interfaces/IHttpRequest.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Server/PTWServerSettings.h"
#include "PTWGameLiftClientSubsystem.generated.h"


namespace GameLiftClientText
{
	extern const FText InvalidWebsocket;
	extern const FText DisconnectedWebsocket;
	extern const FText InvalidLocalUniqueId;
	
	extern const FText ProcessRequestFailed;
	extern const FText JsonParseFailed;
	
	extern const FText CreatePlayerSessionFailed;
	extern const FText StartMatchmakingFailed;
	extern const FText AcceptMatchmakingFailed;
}

class FJsonObject;
struct FOnlineSessionSearchResultBP;
class UPTWWebSocketClientSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameLiftSessionSearchComplete, const TArray<FPTWGameSessionListsTable>&, SearchResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameLiftSessionMessageReceived, const FText&, Message);

/*
 * 클라이언트 전용 로직을 관리하는 게임리프트 서브 시스템입니다.
 */
UCLASS()
class PTW_API UPTWGameLiftClientSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UPTWGameLiftClientSubsystem();
	
	static UPTWGameLiftClientSubsystem* Get(const UObject* WorldContextObject);
	UPTWWebSocketClientSubsystem* GetConnectedWebSocketSubsystem(bool bShowPopup = false) const;
	FString GetValidLocalUniqueId(bool bShowPopup = false) const;
	
	void StartGameSession(FPTWServerSettings& SessionConfig);
	void JoinGameSession(const FString& SteamId, const FString& PlayerSessionId);
	void CreatePlayerSession(const FString& PlayerId, const FString& GameSessionId);
	void SearchGameSessions();
	void StartMatchmaking();
	void StopMatchmaking(const FString& TicketId);
	void AcceptMatchmaking(const FString& TicketId, bool bIsAccepted);
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void FindByIdAndJoinSession(const FString& SteamId, const FString& Options, int32 LoopCount = 3);
	void CreatePlayerSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void SearchGameSessions_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void StartMatchmaking_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void StopMatchmaking_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void AcceptMatchmaking_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

public:
	FOnGameLiftSessionSearchComplete OnSessionSearchComplete;
	FOnGameLiftSessionMessageReceived OnGameLiftSessionMessageReceived;
	
private:
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	
};
