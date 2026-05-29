#include "PTWGameLiftClientSubsystem.h"
#include "HttpModule.h"
#include "PTWMatchmakingSubsystem.h"
#include "UObject/Object.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/IHttpResponse.h"
#include "API/PTWAPIData.h"
#include "PTWSteamSessionSubsystem.h"
#include "PTWWebSocketClientSubsystem.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "Utilities/PTWJsonUtility.h"
#include "Debug/PTWLogCategorys.h"


#define LOCTEXT_NAMESPACE "GameLiftClient"
namespace GameLiftClientText
{
	const FText InvalidWebsocket			= LOCTEXT("InvalidWebsocket", "[웹소켓 오류]\n웹소켓이 유효하지 않아 오류가 발생했습니다.");
	const FText DisconnectedWebsocket		= LOCTEXT("DisconnectedWebsocket", "[웹소켓 오류]\n웹소켓이 연결이 끊어져 오류가 발생했습니다.");
	const FText InvalidLocalUniqueId		= LOCTEXT("InvalidLocalUniqueId", "[스팀 로그인 오류]\n스팀 계정Id를 불러오는데 실패했습니다.");
	
	const FText ProcessRequestFailed		= LOCTEXT("ProcessRequestFailed", "[시스템 오류]\n요청 시스템 오류가 발생했습니다.");
	const FText JsonParseFailed				= LOCTEXT("JsonParseFailed", "[시스템 오류]\n서버 데이터를 해석하는데 실패했습니다.");
	
	const FText CreatePlayerSessionFailed	= LOCTEXT("CreatePlayerSessionFailed", "[플레이어 세션 오류]\n서버를 접속하기 위한 플레이어 세션 생성에 실패했습니다.");
	const FText StartMatchmakingFailed		= LOCTEXT("StartMatchmakingFailed", "[매치메이킹 오류]\n매치메이킹 등록에 실패했습니다.");
	const FText AcceptMatchmakingFailed		= LOCTEXT("AcceptMatchmakingFailed", "[매치메이킹 오류]\n매치메이킹 수락에 실패했습니다.");
}
#undef LOCTEXT_NAMESPACE

UPTWGameLiftClientSubsystem::UPTWGameLiftClientSubsystem()
{
}

UPTWGameLiftClientSubsystem* UPTWGameLiftClientSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (IsValid(World))
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UPTWGameLiftClientSubsystem>();
		}
	}
	return nullptr;
}

UPTWWebSocketClientSubsystem* UPTWGameLiftClientSubsystem::GetConnectedWebSocketSubsystem(bool bShowPopup) const
{
	UPTWWebSocketClientSubsystem* WebSocketSubsystem = UPTWWebSocketClientSubsystem::Get(this);
	if (IsValid(WebSocketSubsystem))
	{
		if (!WebSocketSubsystem->IsWebSocketValid())
		{
			if (bShowPopup) 
			{
				OnGameLiftSessionMessageReceived.Broadcast(GameLiftClientText::InvalidWebsocket);
			}
			return nullptr;
		}
		if (!WebSocketSubsystem->IsWebSocketConnected())
		{
			if (bShowPopup)
			{
				OnGameLiftSessionMessageReceived.Broadcast(GameLiftClientText::DisconnectedWebsocket);
			}
			return nullptr;
		}
		return WebSocketSubsystem;
	}
	return nullptr;
}

FString UPTWGameLiftClientSubsystem::GetValidLocalUniqueId(bool bShowPopup) const
{
	if (UPTWGameInstance* GI = GetWorld()->GetGameInstance<UPTWGameInstance>())
	{
		if (!GI->LocalUniqueId.IsEmpty())
		{
			return GI->LocalUniqueId;
		}
	}

	if (bShowPopup)
	{
		OnGameLiftSessionMessageReceived.Broadcast(GameLiftClientText::InvalidLocalUniqueId);
	}
	return FString();
}

void UPTWGameLiftClientSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPTWGameLiftClientSubsystem::Deinitialize()
{
	if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
	{
		if (IOnlineSessionPtr SessionInterface = SteamSessionSubsystem->GetSessionInterface())
		{
			SessionInterface->ClearOnFindSessionsCompleteDelegates(this);
		}
	}

	Super::Deinitialize();
}

void UPTWGameLiftClientSubsystem::StartGameSession(FPTWServerSettings& SessionConfig)
{
	UPTWWebSocketClientSubsystem* WebSocketSubsystem = UPTWWebSocketClientSubsystem::Get(this);
	if (!IsValid(WebSocketSubsystem) || !WebSocketSubsystem->IsWebSocketValid() || !WebSocketSubsystem->IsWebSocketConnected()) return;
	
	FString LocalUniqueId;
	if (UPTWGameInstance* GI = GetWorld()->GetGameInstance<UPTWGameInstance>())
	{
		LocalUniqueId = GI->LocalUniqueId;
	}
	if (LocalUniqueId.IsEmpty()) return;
	
	FString MaxRounds = TEXT("Long");
	
	TArray<TSharedPtr<FJsonValue>> ReservedAccountIds;
	ReservedAccountIds.Add(MakeShared<FJsonValueString>(LocalUniqueId));
	
	TSharedPtr<FJsonObject> RequestJsonObject = MakeShareable(new FJsonObject());
	RequestJsonObject->SetArrayField(TEXT("reservedAccountIds"), ReservedAccountIds);
	RequestJsonObject->SetStringField(TEXT("name"), SessionConfig.ServerName);
	// RequestJsonObject->SetStringField(TEXT("maximumPlayerSessionCount"), 123);
	RequestJsonObject->SetStringField(TEXT("maxRoundType"), MaxRounds);
	
	FString Content;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Content);
	FJsonSerializer::Serialize(RequestJsonObject.ToSharedRef(), JsonWriter);
	
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	// Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::CreateGameSession_Response);
	
	// TODO: 연결 재설정 필요함.
	const FString APIURL = FPaths::Combine(FString(GAMELIFT_CLIENTAPI_ENDPOINT), FString());
	
	Request->SetURL(APIURL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(Content);

	if (Request->ProcessRequest())
	{
		UE_LOG(Log_GameLift, Display, TEXT("[게임세션 생성요청] 게임리프트 게임세션 생성요청 전송 완료"));
	}
	else
	{
		OnGameLiftSessionMessageReceived.Broadcast(GameLiftClientText::ProcessRequestFailed);
		UE_LOG(Log_GameLift, Error, TEXT("[게임세션 생성요청] 게임리프트 게임세션 생성요청 전송 실패"));
	}
}

void UPTWGameLiftClientSubsystem::JoinGameSession(const FString& SteamId, const FString& PlayerSessionId)
{
	FString ConnectURL = FString::Printf(TEXT("?PlayerSessionId=%s"), *PlayerSessionId);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *ConnectURL);
		
	FindByIdAndJoinSession(SteamId, ConnectURL);
}

void UPTWGameLiftClientSubsystem::CreatePlayerSession(const FString& PlayerId, const FString& GameSessionId)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::CreatePlayerSession_Response);
	
	const FString APIURL = FPaths::Combine(FString(GAMELIFT_CLIENTAPI_ENDPOINT), GameLift::ClientAPI::Routes::CreatePlayerSession);
	Request->SetURL(APIURL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	TMap<FString, FString> Params = {
		{ TEXT("playerId"), PlayerId },
		{ TEXT("gameSessionId"), GameSessionId }
	};
	
	const FString Content = UPTWJsonUtility::MakeHTTPRequestBody(Params);
	
	Request->SetContentAsString(Content);
	if (Request->ProcessRequest())
	{
		UE_LOG(Log_GameLift, Display, TEXT("[플레이어세션 생성요청] 게임리프트 게임세션 - 플레이어세션 생성요청 전송완료 (GameSessionId:%s)"), *GameSessionId);
	}
	else
	{
		OnGameLiftSessionMessageReceived.Broadcast(GameLiftClientText::ProcessRequestFailed);
		UE_LOG(Log_GameLift, Error, TEXT("[플레이어세션 생성요청] 게임리프트 게임세션 - 플레이어세션 생성요청 전송실패 (GameSessionId:%s)"), *GameSessionId);
	}
}

void UPTWGameLiftClientSubsystem::CreatePlayerSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	
	if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		OnGameLiftSessionMessageReceived.Broadcast(GameLiftClientText::CreatePlayerSessionFailed);
		UE_LOG(Log_GameLift, Error, TEXT("[플레이어세션 생성응답] 게임리프트 게임세션 - 플레이어세션 생성실패 응답 (Code:%d)"), Response->GetResponseCode());
		return;
	}
	UE_LOG(Log_GameLift, Display, TEXT("[플레이어세션 생성응답] 게임리프트 게임세션 - 플레이어세션 생성성공 응답 (Code:%d)"), Response->GetResponseCode());
	
	FString PlayerSessionIdName = TEXT("PlayerSessionId");
	FString SteamIdName = TEXT("SteamId");
	
	TArray<FString> FieldsToExtract = { PlayerSessionIdName, SteamIdName };
	TMap<FString, FString> SessionData = UPTWJsonUtility::ExtractFieldsFromHTTPData(Response->GetContentAsString(), FieldsToExtract);
	if (SessionData.Contains(PlayerSessionIdName) && SessionData.Contains(SteamIdName))
	{
		// ("steam.%s:%s?PlayerSessionId=%s")
		JoinGameSession(SessionData[SteamIdName], SessionData[PlayerSessionIdName]);
	}
	else
	{
		OnGameLiftSessionMessageReceived.Broadcast(GameLiftClientText::JsonParseFailed);
	}
}

void UPTWGameLiftClientSubsystem::SearchGameSessions()
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::SearchGameSessions_Response);
	
	const FString APIURL = FPaths::Combine(FString(GAMELIFT_CLIENTAPI_ENDPOINT), GameLift::ClientAPI::Routes::SearchGameSessions);
	Request->SetURL(APIURL);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->ProcessRequest();
}

void UPTWGameLiftClientSubsystem::SearchGameSessions_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogTemp, Error, TEXT("HTTP Request failed!"));
		return;
	}
	
	TArray<FPTWGameSessionListsTable> GameSessionLists;
	// UPTWJsonUtility::ParseHTTPDataToArray(Response->GetContentAsString(), GameSessionLists);
	if (GameSessionLists.IsEmpty()) return;
	
	if (OnSessionSearchComplete.IsBound())
	{
		OnSessionSearchComplete.Broadcast(GameSessionLists);
	}
}

void UPTWGameLiftClientSubsystem::StartMatchmaking()
{
	UPTWWebSocketClientSubsystem* WebSocketSubsystem = GetConnectedWebSocketSubsystem(true);
	if (!IsValid(WebSocketSubsystem)) return;
	
	FString LocalUniqueId = GetValidLocalUniqueId(true);
	if (LocalUniqueId.IsEmpty()) return;
	
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::StartMatchmaking_Response);
	
	const FString APIURL = FPaths::Combine(FString(GAMELIFT_CLIENTAPI_ENDPOINT), GameLift::ClientAPI::Routes::StartMatchmaking);
	
	TSharedPtr<FJsonObject> RequestJsonObject = MakeShareable(new FJsonObject());
	
	TSharedPtr<FJsonObject> LatencyMap = MakeShared<FJsonObject>();
	LatencyMap->SetNumberField(TEXT("ap-northeast-2"), 45);
	RequestJsonObject->SetObjectField(TEXT("latencyMap"), LatencyMap);
	RequestJsonObject->SetStringField(TEXT("accountId"), LocalUniqueId);
	
	FString Content;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Content);
	FJsonSerializer::Serialize(RequestJsonObject.ToSharedRef(), JsonWriter);
	
	Request->SetURL(APIURL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(Content);
	
	if (Request->ProcessRequest())
	{
		UE_LOG(Log_DynamoDB, Display, TEXT("[매치메이킹 등록요청] 매치메이킹 등록요청 전송성공"));
	}
	else
	{
		OnGameLiftSessionMessageReceived.Broadcast(GameLiftClientText::ProcessRequestFailed);
		UE_LOG(Log_DynamoDB, Error, TEXT("[매치메이킹 등록요청] 매치메이킹 등록요청 전송실패"));
	}
}

void UPTWGameLiftClientSubsystem::StartMatchmaking_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		OnGameLiftSessionMessageReceived.Broadcast(GameLiftClientText::StartMatchmakingFailed);
		UE_LOG(Log_DynamoDB, Error, TEXT("[매치메이킹 등록응답] 매치메이킹 등록실패 응답"));
		return;
	}
	UE_LOG(Log_DynamoDB, Display, TEXT("[매치메이킹 등록응답] 매치메이킹 등록성공 응답"));

	const FString SerializedString = Response->GetContentAsString();
	const FString TICKETID = TEXT("TicketId");
	const FString EVENTTYPE = TEXT("EventType");
	const FString EVENTTIME = TEXT("EventTime");

	TArray<FString> FieldsToExtract = { TICKETID, EVENTTYPE, EVENTTIME };
	TMap<FString, FString> ParsedData = UPTWJsonUtility::ExtractFieldsFromHTTPData(SerializedString, FieldsToExtract);
	
	// CANCELLED | COMPLETED | FAILED | PLACING | QUEUED | REQUIRES_ACCEPTANCE | SEARCHING | TIMED_OUT
	if (ParsedData.Contains(TICKETID) && ParsedData.Contains(EVENTTYPE) && ParsedData.Contains(EVENTTIME) && !EVENTTIME.IsEmpty())
	{
		if (UPTWMatchmakingSubsystem* MatchmakingSubsystem = UPTWMatchmakingSubsystem::Get(this))
		{
			MatchmakingSubsystem->ShowWidget();
			MatchmakingSubsystem->InitMatchmaking(true);
			MatchmakingSubsystem->MatchmakingEvent.StartTime = FCString::Atoi64(*ParsedData[EVENTTIME]);
			MatchmakingSubsystem->HandleReceivePayload(SerializedString);
		}
	}
	else
	{
		OnGameLiftSessionMessageReceived.Broadcast(GameLiftClientText::JsonParseFailed);
	}
}

void UPTWGameLiftClientSubsystem::StopMatchmaking(const FString& TicketId)
{
	if (TicketId.IsEmpty()) return;
	UPTWWebSocketClientSubsystem* WebSocketSubsystem = GetConnectedWebSocketSubsystem(true);
	if (!IsValid(WebSocketSubsystem)) return;
	
	FString LocalUniqueId = GetValidLocalUniqueId(true);
	if (LocalUniqueId.IsEmpty()) return;
	
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::StopMatchmaking_Response);
	
	const FString APIURL = FPaths::Combine(FString(GAMELIFT_CLIENTAPI_ENDPOINT), GameLift::ClientAPI::Routes::StopMatchmaking);
	
	TSharedPtr<FJsonObject> RequestJsonObject = MakeShareable(new FJsonObject());
	RequestJsonObject->SetStringField(TEXT("accountId"), LocalUniqueId);
	RequestJsonObject->SetStringField(TEXT("ticketId"), TicketId);
	
	FString Content;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Content);
	FJsonSerializer::Serialize(RequestJsonObject.ToSharedRef(), JsonWriter);
	
	Request->SetURL(APIURL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(Content);
	
	if (Request->ProcessRequest())
	{
		UE_LOG(Log_DynamoDB, Display, TEXT("[매치메이킹 취소요청] 매치메이킹 취소요청 전송성공"));
	}
	else
	{
		UE_LOG(Log_DynamoDB, Error, TEXT("[매치메이킹 취소요청] 매치메이킹 취소요청 전송실패"));
	}
}

void UPTWGameLiftClientSubsystem::StopMatchmaking_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(Log_DynamoDB, Display, TEXT("[매치메이킹 취소응답] 매치메이킹 취소성공 응답"));
	}
	else
	{
		UE_LOG(Log_DynamoDB, Error, TEXT("[매치메이킹 취소응답] 매치메이킹 취소실패 응답"));
	}
	
	if (UPTWMatchmakingSubsystem* MatchmakingSubsystem = UPTWMatchmakingSubsystem::Get(this))
	{
		MatchmakingSubsystem->InitMatchmaking();
		MatchmakingSubsystem->HideWidget();
	}
}

void UPTWGameLiftClientSubsystem::FindByIdAndJoinSession(const FString& SteamId, const FString& Options, int32 LoopCount)
{
	UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this);
	if(!IsValid(SteamSessionSubsystem)) return;
	
	IOnlineSessionPtr SessionInterface = SteamSessionSubsystem->GetSessionInterface();
	if(!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

	TSharedPtr<FOnlineSessionSearch> DedicatedSessionSearch = MakeShareable(new FOnlineSessionSearch());
	DedicatedSessionSearch->bIsLanQuery = false;
	DedicatedSessionSearch->MaxSearchResults = 10000;
	DedicatedSessionSearch->QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);
	DedicatedSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, false, EOnlineComparisonOp::Equals);

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateWeakLambda(this, [=, this](bool bWasSuccessful)
		{
			if(SessionInterface.IsValid())
			{
				SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
			}
			
			if (bWasSuccessful && DedicatedSessionSearch.IsValid())
			{
				UE_LOG(Log_Steam, Display, TEXT("[게임세션 탐색응답] 스팀게임세션 탐색성공 응답"));
				for (const FOnlineSessionSearchResult& SearchResult : DedicatedSessionSearch->SearchResults)
				{
					FString TargetSessionId = SearchResult.Session.SessionInfo.IsValid() ? 
					SearchResult.Session.SessionInfo->GetSessionId().ToString() : TEXT("");
					
					if (TargetSessionId.IsEmpty() || SteamId != TargetSessionId) continue;
					
					SteamSessionSubsystem->JoinGameSession(FOnlineSessionSearchResultBP(SearchResult), Options);
					return;
				}
			}
			else
			{
				UE_LOG(Log_Steam, Error, TEXT("[게임세션 탐색응답] 스팀게임세션 탐색실패 응답"));
			}
			// 가끔 스팀에서 세션 탐색에 실패하면 재시도
			if (LoopCount > 0)
			{
				FTimerHandle TempTimerHandle;
				GetWorld()->GetTimerManager().SetTimer(TempTimerHandle, [=, this]()
				{
					FindByIdAndJoinSession(SteamId, Options, LoopCount - 1);
				}, 2.0f, false);
			}
		})
	);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), DedicatedSessionSearch.ToSharedRef()))
	{
		UE_LOG(Log_Steam, Display, TEXT("[게임세션 탐색요청] 스팀게임세션 탐색요청 전송완료"));
	}
	else
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		OnGameLiftSessionMessageReceived.Broadcast(SteamSessionText::ProcessRequestFailed);
		UE_LOG(Log_Steam, Error, TEXT("[게임세션 탐색요청] 스팀게임세션 탐색요청 전송실패"));
	}
}

void UPTWGameLiftClientSubsystem::AcceptMatchmaking(const FString& TicketId, bool bIsAccepted)
{
	UPTWWebSocketClientSubsystem* WebSocketSubsystem = GetConnectedWebSocketSubsystem(true);
	if (!IsValid(WebSocketSubsystem)) return;

	FString LocalUniqueId = GetValidLocalUniqueId(true);
	if (LocalUniqueId.IsEmpty()) return;
	
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::AcceptMatchmaking_Response);
	
	const FString APIURL = FPaths::Combine(FString(GAMELIFT_CLIENTAPI_ENDPOINT), GameLift::ClientAPI::Routes::AcceptMatchmaking);
	
	TSharedPtr<FJsonObject> RequestJsonObject = MakeShareable(new FJsonObject());
	RequestJsonObject->SetStringField(TEXT("ticketId"), TicketId);
	RequestJsonObject->SetStringField(TEXT("accountId"), LocalUniqueId);
	RequestJsonObject->SetBoolField(TEXT("isAccepted"), bIsAccepted);
	
	FString Content;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Content);
	FJsonSerializer::Serialize(RequestJsonObject.ToSharedRef(), JsonWriter);
	
	Request->SetURL(APIURL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(Content);
	
	if (Request->ProcessRequest())
	{
		UE_LOG(Log_DynamoDB, Display, TEXT("[매치메이킹 수락요청] 매치메이킹 수락요청 전송성공"));
	}
	else
	{
		OnGameLiftSessionMessageReceived.Broadcast(GameLiftClientText::ProcessRequestFailed);
		UE_LOG(Log_DynamoDB, Error, TEXT("[매치메이킹 수락요청] 매치메이킹 수락요청 전송실패"));
	}
}

void UPTWGameLiftClientSubsystem::AcceptMatchmaking_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		OnGameLiftSessionMessageReceived.Broadcast(GameLiftClientText::AcceptMatchmakingFailed);
		UE_LOG(Log_DynamoDB, Error, TEXT("[매치메이킹 수락응답] 매치메이킹 수락실패 응답"));
		return;
	}
	UE_LOG(Log_DynamoDB, Display, TEXT("[매치메이킹 수락응답] 매치메이킹 수락성공 응답"));
}
