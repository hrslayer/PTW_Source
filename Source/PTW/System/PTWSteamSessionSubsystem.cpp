#include "PTWSteamSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Online/OnlineSessionNames.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "System/Server/PTWServerSettings.h"
#include "Debug/PTWLogCategorys.h"
#include "Interfaces/OnlineIdentityInterface.h"

#define LOCTEXT_NAMESPACE "SteamSession"
namespace SteamSessionText
{
	const FText ProcessRequestFailed			= LOCTEXT("ProcessRequestFailed", "[시스템 오류]\n요청 시스템 오류가 발생했습니다.");
	const FText SteamAPILoginFailed				= LOCTEXT("SteamAPILoginFailed", "[로그인 오류]\n스팀 로그인에 실패했습니다.\n현재 스팀클라이언트가 로그인되어 있는지 확인해주세요.");
	
	const FText CreateGameSessionFailed			= LOCTEXT("CreateGameSessionFailed", "[게임 세션 오류]\n네트워크 문제로 스팀세션 생성에 실패했습니다.");
	const FText JoinGameSessionFailed			= LOCTEXT("JoinGameSessionFailed", "[게임 세션 오류]\n스팀 세션 참여에 실패했습니다.");
	const FText JoinGameSessionFailed_Parsing	= LOCTEXT("JoinGameSessionFailed_Parse", "[시스템 오류]\n스팀 세션 데이터 해석에 실패했습니다.");
	const FText SearchForGameSessionsFailed		= LOCTEXT("SearchForGameSessionsFailed", "[게임 세션 오류]\n게임 세션 탐색에 실패했습니다.");
}
#undef LOCTEXT_NAMESPACE

UPTWSteamSessionSubsystem* UPTWSteamSessionSubsystem::Get(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, 
		EGetWorldErrorMode::LogAndReturnNull) : nullptr)
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UPTWSteamSessionSubsystem>();
		}
	}
	return nullptr;
}

IOnlineSessionPtr UPTWSteamSessionSubsystem::GetSessionInterface() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(World))
		{
			if (IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface())
			{
				return SessionInterface;
			}
		}
	}
	return nullptr;
}

IOnlineIdentityPtr UPTWSteamSessionSubsystem::GetIdentityInterface() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(World))
		{
			if (IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface())
			{
				return IdentityInterface;
			}
		}
	}
	return nullptr;
}

FString UPTWSteamSessionSubsystem::GetSteamServerId() const
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		FNamedOnlineSession* CurrentGameSession = SessionInterface->GetNamedSession(NAME_GameSession);
		if (CurrentGameSession && CurrentGameSession->SessionInfo.IsValid())
		{
			const FUniqueNetId& SessionId = CurrentGameSession->SessionInfo->GetSessionId();

			return SessionId.IsValid() ? SessionId.ToString() : FString();
		}
	}
	
	return FString();
}

void UPTWSteamSessionSubsystem::CreateGameSession(const FPTWServerSettings& ServerSettings)
{
	if (!ServerSettings.IsValid()) return;
	if (UPTWGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UPTWGameInstance>() : nullptr)
	{
		GI->ServerSettings = ServerSettings;
	}
	
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if(!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	
	// 이미 생성된 세션이 존재할 경우 파괴
	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}
	
	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->bIsLANMatch = false;											// Lan 연결 사용 여부
	SessionSettings->bShouldAdvertise = true;										// 공개 여부: 검색 노출 및 친구 초대 가능
	SessionSettings->bAllowJoinInProgress = true;									// 중간 난입 허용 여부
	SessionSettings->NumPublicConnections = ServerSettings.MaxPlayerCount;   
    
	SessionSettings->bIsDedicated = ServerSettings.bIsDedicatedServer;				// Dedicated Serer 여부
	SessionSettings->bUsesPresence = !ServerSettings.bIsDedicatedServer;				// 스팀 상태 정보(Presence)
	SessionSettings->bAllowJoinViaPresence = !ServerSettings.bIsDedicatedServer;		// 스팀 친구 참여
	SessionSettings->bUseLobbiesIfAvailable = !ServerSettings.bIsDedicatedServer;
	
	const FString& ServerName = ServerSettings.ServerName;
	const FString& ServerType = FPTWServerSettings::ServerTypeToString(ServerSettings.ServerType);
	const FString& RoundType = FPTWServerSettings::RoundTypeToString(ServerSettings.RoundType);
	
	FString ErrorMessage;
	if (!ServerName.IsEmpty() && !ServerType.IsEmpty() && !RoundType.IsEmpty())
	{
		CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
			FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));
	
		if (ServerSettings.bIsDedicatedServer == false)
		{
			SessionSettings->Set(FPTWServerSettings::Key_ServerName, ServerName, EOnlineDataAdvertisementType::ViaOnlineService);
			SessionSettings->Set(FPTWServerSettings::Key_ServerType, ServerType, EOnlineDataAdvertisementType::ViaOnlineService);
			SessionSettings->Set(FPTWServerSettings::Key_RoundType, RoundType, EOnlineDataAdvertisementType::ViaOnlineService);
		}
    
		if (SessionInterface->CreateSession(0, NAME_GameSession, *SessionSettings))
		{
			UE_LOG(Log_Steam, Display, TEXT("[게임세션 생성요청] 스팀게임세션 생성요청 전송완료"));
			return;
		}

		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}
	else
	{
		ErrorMessage = FString::Printf(TEXT("[데이터 누락] ServerName: %s, ServerType: %s, RoundType: %s"), 
			*ServerName, *ServerType, *RoundType);
	}
	if (!ErrorMessage.IsEmpty())
	{
		ErrorMessage = TEXT(", ") + ErrorMessage;
	}
	UE_LOG(Log_Steam, Error, TEXT("[게임세션 생성요청] 스팀게임세션 생성요청 전송실패%s"), *ErrorMessage);
	
	// 서버는 AlarmUI 미표시
	#if !UE_SERVER
		OnSteamSessionMessageReceived.Broadcast(SteamSessionText::ProcessRequestFailed);
	#endif
}

void UPTWSteamSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if(!SessionInterface.IsValid()) return;
	
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	if (bWasSuccessful)
	{
		UE_LOG(Log_Steam, Display, TEXT("[게임세션 생성응답] 스팀게임세션 생성성공 응답"));
		SteamServerTravel("Staging");
	}
	else
	{
		OnSteamSessionMessageReceived.Broadcast(SteamSessionText::CreateGameSessionFailed);
		UE_LOG(Log_Steam, Error, TEXT("[게임세션 생성응답] 스팀게임세션 생성실패 응답"));
	}
}

void UPTWSteamSessionSubsystem::JoinGameSession(const FOnlineSessionSearchResultBP& BPSearchResult, const FString Options)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	const FOnlineSessionSearchResult& SearchResult = BPSearchResult.OnlineSessionSearchResult;
	
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete, Options));
	
	if (SessionInterface->JoinSession(0, NAME_GameSession, SearchResult))
	{
		UE_LOG(Log_Steam, Display, TEXT("[게임세션 접속요청] 스팀게임세션 접속요청 전송완료"));
	}
	else
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		OnSteamSessionMessageReceived.Broadcast(SteamSessionText::ProcessRequestFailed);
		UE_LOG(Log_Steam, Error, TEXT("[게임세션 접속요청] 스팀게임세션 접속요청 전송실패"));
	}
}

void UPTWSteamSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result, const FString Options)
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(Log_Steam, Error, TEXT("[게임세션 접속응답] 스팀게임세션 접속실패 응답 (Code: %d)"), (int32)Result);
		OnSteamSessionMessageReceived.Broadcast(SteamSessionText::JoinGameSessionFailed);
		return;
	}
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	
	FString ConnectString;
	if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
	{
		UE_LOG(Log_Steam, Display, TEXT("[게임세션 접속응답] 스팀게임세션 접속성공 응답"));
		ConnectString += Options; 
		if (APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
		{
			PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
		}
	}
	else
	{
		OnSteamSessionMessageReceived.Broadcast(SteamSessionText::JoinGameSessionFailed_Parsing);
		UE_LOG(Log_Steam, Error, TEXT("[게임세션 접속실패] 세션 접속 정보 해석 실패. 유효하지 않은 세션 또는 네트워크 상태 확인"));
	}
}

void UPTWSteamSessionSubsystem::FindGameSession()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if(!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	
	SessionSearchQueue.Empty();
	BPSearchResults.Reset();
	
	TSharedPtr<FOnlineSessionSearch> ListenSessionSearch = MakeShareable(new FOnlineSessionSearch());
	ListenSessionSearch->bIsLanQuery = false;
	ListenSessionSearch->MaxSearchResults = 100;
	ListenSessionSearch->QuerySettings.Set(SEARCH_DEDICATED_ONLY, false, EOnlineComparisonOp::Equals);
	ListenSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	SessionSearchQueue.Enqueue(ListenSessionSearch);
	
	TSharedPtr<FOnlineSessionSearch> DedicatedSessionSearch = MakeShareable(new FOnlineSessionSearch());
	DedicatedSessionSearch->bIsLanQuery = false;
	DedicatedSessionSearch->MaxSearchResults = 100;
	DedicatedSessionSearch->QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);
	DedicatedSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, false, EOnlineComparisonOp::Equals);
	SessionSearchQueue.Enqueue(DedicatedSessionSearch);
	
	SearchForGameSessions();
}

void UPTWSteamSessionSubsystem::SearchForGameSessions()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if(!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	
	if (SessionSearchQueue.IsEmpty()) return;
	
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	SessionSearchQueue.Peek(SessionSearch);
	
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)
	);
	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef()))
	{
		UE_LOG(Log_Steam, Display, TEXT("[게임세션 탐색요청] 스팀게임세션 탐색요청 전송완료"));
	}
	else
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		OnSteamSessionMessageReceived.Broadcast(SteamSessionText::ProcessRequestFailed);
		UE_LOG(Log_Steam, Error, TEXT("[게임세션 탐색요청] 스팀게임세션 탐색요청 전송실패"));
	}
}

void UPTWSteamSessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if(!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	
	if(SessionSearchQueue.IsEmpty()) return;
	
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	SessionSearchQueue.Dequeue(SessionSearch);
	
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		UE_LOG(Log_Steam, Display, TEXT("[게임세션 탐색응답] 스팀게임세션 탐색성공 응답"));
		TArray<FOnlineSessionSearchResultBP> BPSearchResultInstances;
		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			BPSearchResultInstances.Add(FOnlineSessionSearchResultBP(SearchResult));
		}
		
		OnSessionSearchComplete.Broadcast(BPSearchResultInstances);
		
		BPSearchResults += BPSearchResultInstances;
		// 마지막 탐색에서 리스트 반환
		if(SessionSearchQueue.IsEmpty())
		{
			OnAllSessionSearchFinished.Broadcast();
		}
		else
		{
			SearchForGameSessions();
		}
	}
	else
	{
		OnSteamSessionMessageReceived.Broadcast(SteamSessionText::SearchForGameSessionsFailed);
		UE_LOG(Log_Steam, Error, TEXT("[게임세션 탐색응답] 스팀게임세션 탐색실패 응답"));
	}
}

void UPTWSteamSessionSubsystem::DestroySession()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid()) return;
	
	DestroySessionDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete));
		
	if (SessionInterface->DestroySession(NAME_GameSession))
	{
		UE_LOG(Log_Steam, Display, TEXT("[게임세션 파괴요청] 스팀게임세션 파괴요청 전송완료"));
	}
	else
	{
		UE_LOG(Log_Steam, Error, TEXT("[게임세션 파괴요청] 스팀게임세션 파괴요청 전송실패"));
		OnSteamSessionMessageReceived.Broadcast(SteamSessionText::ProcessRequestFailed);
		OnDestroySessionComplete(NAME_GameSession, false);
	}
}

void UPTWSteamSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionDelegateHandle);
	}
	
	if (bWasSuccessful)
	{
		UE_LOG(Log_Steam, Display, TEXT("[게임세션 파괴응답] 스팀게임세션 파괴성공 응답"));
	}
	else
	{
		UE_LOG(Log_Steam, Error, TEXT("[게임세션 파괴응답] 스팀게임세션 파괴실패 응답"));
	}
	
	if (APlayerController* PC = GetGameInstance() ? GetGameInstance()->GetFirstLocalPlayerController() : nullptr)
	{
		PC->ClientTravel(TEXT("MainMenu?closed"), TRAVEL_Absolute);
	}
}

void UPTWSteamSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (!IsRunningDedicatedServer())
	{
		if (const IOnlineIdentityPtr IdentityInterface = GetIdentityInterface())
		{
			TSharedPtr<const FUniqueNetId> UniqueId = IdentityInterface->GetUniquePlayerId(0);
			if (UniqueId.IsValid())
			{
				OnLoginComplete(0, true, *UniqueId.ToSharedRef(), TEXT(""));
			}
			else
			{
				LoginCompletedDelegateHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(0, 
					FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnLoginComplete));
			}
		}
		else
		{
			PostWorldInitializationDelegateHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ThisClass::HandlePostWorldInitialization);
		}
	}
	
	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &ThisClass::HandleNetworkFailure);
	}
	
	SessionSearchQueue.Empty();
	BPSearchResults.Reset();
}

void UPTWSteamSessionSubsystem::Deinitialize()
{
	if (GEngine)
	{
		GEngine->OnNetworkFailure().RemoveAll(this);
	}
	if (IOnlineIdentityPtr IdentityInterface = GetIdentityInterface())
	{
		IdentityInterface->ClearOnLoginCompleteDelegates(0, this);
	}
	if (IOnlineSessionPtr SessionInterface = GetSessionInterface())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegates(this);
		SessionInterface->ClearOnFindSessionsCompleteDelegates(this);
		SessionInterface->ClearOnJoinSessionCompleteDelegates(this);
		SessionInterface->ClearOnDestroySessionCompleteDelegates(this);
	}
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	
	Super::Deinitialize();
}

void UPTWSteamSessionSubsystem::HandlePostWorldInitialization(UWorld* World, const UWorld::InitializationValues Ivs)
{
	if (!IsValid(World) || !World->IsGameWorld()) return;
	
	if (PostWorldInitializationDelegateHandle.IsValid())
	{
		FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitializationDelegateHandle);
	}
	
	if (const IOnlineIdentityPtr IdentityInterface = GetIdentityInterface())
	{
		TSharedPtr<const FUniqueNetId> UniqueId = IdentityInterface->GetUniquePlayerId(0);
		if (UniqueId.IsValid())
		{
			OnLoginComplete(0, true, *UniqueId.ToSharedRef(), TEXT(""));
		}
		else
		{
			LoginCompletedDelegateHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(0, 
				FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnLoginComplete));
		}
	}
}

void UPTWSteamSessionSubsystem::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UniqueId, const FString& Error)
{
	IOnlineIdentityPtr IdentityInterface = GetIdentityInterface();
	if (IdentityInterface.IsValid() && LoginCompletedDelegateHandle.IsValid())
	{
		IdentityInterface->ClearOnLoginCompleteDelegate_Handle(0, LoginCompletedDelegateHandle);
		LoginCompletedDelegateHandle.Reset();
	}
	if (!bWasSuccessful || !UniqueId.IsValid())
	{
		OnSteamSessionMessageReceived.Broadcast(SteamSessionText::SteamAPILoginFailed);
		return;
	}
	
	if (UPTWGameInstance* GI = GetWorld()->GetGameInstance<UPTWGameInstance>())
	{
		GI->LocalUniqueId = UniqueId.ToString();
		GI->OnLocalUniqueIdCreated.Broadcast(GI->LocalUniqueId);
	}
}

void UPTWSteamSessionSubsystem::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(Log_Steam, Error, TEXT("[게임세션 오류] 스팀게임세션 끊김발생"));
	DestroySession();
}

void UPTWSteamSessionSubsystem::SteamServerTravel(const FName& MapName) const
{
	if (IsRunningDedicatedServer())
	{
		GetWorld()->ServerTravel(MapName.ToString());
		return;
	}
	
	FString Options = TEXT("?listen");
	UGameplayStatics::OpenLevel(this, MapName, true, Options);
}
