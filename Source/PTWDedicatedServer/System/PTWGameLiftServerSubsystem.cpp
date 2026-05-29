#include "PTWGameLiftServerSubsystem.h"
#include "PTW/Debug/PTWLogCategorys.h"
#include "API/PTWAPIData.h"
#include "GameLiftServerSDK.h"
#include "HttpModule.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "Debug/PTWLogCategorys.h"
#include "Interfaces/IHttpResponse.h"
#include "PTW/System/PTWSteamSessionSubsystem.h"


UPTWGameLiftServerSubsystem::UPTWGameLiftServerSubsystem()
{
}

UPTWGameLiftServerSubsystem* UPTWGameLiftServerSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (IsValid(World))
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UPTWGameLiftServerSubsystem>();
		}
	}
	return nullptr;
}

IOnlineSessionPtr UPTWGameLiftServerSubsystem::GetSessionInterface() const
{
	if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
	{
		return SteamSessionSubsystem->GetSessionInterface();
	}
	return nullptr;
}

void UPTWGameLiftServerSubsystem::ActivateGameSession()
{
	UPTWGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UPTWGameInstance>() : nullptr;
	if (!IsValid(GI)) return;
	
	UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this);
	if (!IsValid(SteamSessionSubsystem)) return;
	
	const FString GameSessionId = GameLiftSdkModule->GetGameSessionId().GetResult();
	const FString SteamId = SteamSessionSubsystem->GetSteamServerId();
	
	const FPTWServerSettings& ServerSettings = GI->ServerSettings;
	const FString ServerName = ServerSettings.ServerName;
	const FString RoundType = FPTWServerSettings::RoundTypeToString(ServerSettings.RoundType);
	const FString ServerType = FPTWServerSettings::ServerTypeToString(ServerSettings.ServerType);
	const int32 MaxPlayerCount = ServerSettings.MaxPlayerCount;
	
	FString ErrorMessage;
	if (!GameSessionId.IsEmpty() && !SteamId.IsEmpty() && !RoundType.IsEmpty() && !ServerType.IsEmpty() && 
		!ServerName.IsEmpty() && MaxPlayerCount > 0)
	{
		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::ActivateGameSession_Response);
		const FString APIURL = FPaths::Combine(FString(GAMELIFT_SERVERAPI_ENDPOINT), GameLift::ServerAPI::Routes::ActivateGameSession);
	
		TSharedPtr<FJsonObject> RequestJsonObject = MakeShareable(new FJsonObject());
	
		RequestJsonObject->SetStringField(TEXT("gameSessionId"), GameSessionId);
		RequestJsonObject->SetStringField(TEXT("serverName"), ServerName);
		RequestJsonObject->SetStringField(TEXT("steamId"), SteamId);
		RequestJsonObject->SetStringField(TEXT("roundType"), RoundType);
		RequestJsonObject->SetStringField(TEXT("serverType"), ServerType);
		RequestJsonObject->SetNumberField(TEXT("maxPlayerCount"), MaxPlayerCount);
	
		FString Content;
		TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Content);
		FJsonSerializer::Serialize(RequestJsonObject.ToSharedRef(), JsonWriter);
	
		Request->SetURL(APIURL);
		Request->SetVerb(TEXT("POST"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		Request->SetContentAsString(Content);
	
		if (Request->ProcessRequest())
		{
			UE_LOG(Log_DynamoDB, Display, TEXT("[DB 레코드 생성요청] DynamoDB 레코드 생성요청 전송성공"));
			return;
		}
	}
	else
	{
		ErrorMessage = FString::Printf(
			TEXT("[데이터 누락] GameSessionId: %s, ServerName: %s, SteamId: %s, RoundType: %s, ServerType: %s, MaxPlayerCount: %d")
			, *GameSessionId, *ServerName, *SteamId, *RoundType, *ServerType, MaxPlayerCount);
	}
	if (!ErrorMessage.IsEmpty())
	{
		ErrorMessage = TEXT(", ") + ErrorMessage;
	}
	UE_LOG(Log_DynamoDB, Error, TEXT("[DB 레코드 생성요청] DynamoDB 레코드 생성요청 전송실패%s"), *ErrorMessage);
}

void UPTWGameLiftServerSubsystem::ActivateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(Log_DynamoDB, Display, TEXT("[DB 레코드 생성요청] DynamoDB 레코드 생성성공 응답"));
		GameLiftSdkModule->ActivateGameSession();
		
		if (FTimerManager* TimerManager = GetWorld() ? &GetWorld()->GetTimerManager() : nullptr)
		{
			TimerManager->SetTimer(
				HeartbeatTimerHandle,
				this,
				&ThisClass::HeartbeatGameSession,
				60.0f,
				true);
		}
	}
	else
	{
		UE_LOG(Log_DynamoDB, Error, TEXT("[DB 레코드 생성요청] DynamoDB 레코드 생성실패 응답"));
	}
}

void UPTWGameLiftServerSubsystem::HeartbeatGameSession()
{
	UPTWGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UPTWGameInstance>() : nullptr;
	if (!IsValid(GI)) return;
	
	const FString GameSessionId = GameLiftSdkModule->GetGameSessionId().GetResult();
	
	const FPTWServerSettings& ServerSettings = GI->ServerSettings;
	const FString ServerType = FPTWServerSettings::ServerTypeToString(ServerSettings.ServerType);
	
	FString ErrorMessage;
	if (!GameSessionId.IsEmpty() && !ServerType.IsEmpty())
	{
		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::HeartbeatGameSession_Response);
		const FString APIURL = FPaths::Combine(FString(GAMELIFT_SERVERAPI_ENDPOINT), GameLift::ServerAPI::Routes::HeartbeatGameSession);
	
		TSharedPtr<FJsonObject> RequestJsonObject = MakeShareable(new FJsonObject());
	
		RequestJsonObject->SetStringField(TEXT("gameSessionId"), GameSessionId);
		RequestJsonObject->SetStringField(TEXT("serverType"), ServerType);
	
		FString Content;
		TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Content);
		FJsonSerializer::Serialize(RequestJsonObject.ToSharedRef(), JsonWriter);
	
		Request->SetURL(APIURL);
		Request->SetVerb(TEXT("POST"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		Request->SetContentAsString(Content);
	
		if (Request->ProcessRequest())
		{
			UE_LOG(Log_DynamoDB, Display, TEXT("[DB 속성 갱신요청] DynamoDB 속성 갱신요청 전송성공 [DeleteAt]"));
			return;
		}
	}
	else
	{
		ErrorMessage = FString::Printf(
			TEXT("[데이터 누락] GameSessionId: %s, ServerType: %s"), *GameSessionId, *ServerType);
	}
	if (!ErrorMessage.IsEmpty())
	{
		ErrorMessage = TEXT(", ") + ErrorMessage;
	}
	UE_LOG(Log_DynamoDB, Error, TEXT("[DB 속성 갱신요청] DynamoDB 속성 갱신요청 전송실패 [DeleteAt]%s"), *ErrorMessage);
}

void UPTWGameLiftServerSubsystem::HeartbeatGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(Log_DynamoDB, Display, TEXT("[DB 속성 갱신응답] DynamoDB 속성 갱신성공 응답 [DeleteAt]"));
	}
	else
	{
		UE_LOG(Log_DynamoDB, Error, TEXT("[DB 속성 갱신응답] DynamoDB 속성 갱신실패 응답 [DeleteAt]"));
	}
}

void UPTWGameLiftServerSubsystem::TerminateGameSession()
{
	UPTWGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UPTWGameInstance>() : nullptr;
	if (!IsValid(GI)) return;
	
	const FString GameSessionId = GameLiftSdkModule->GetGameSessionId().GetResult();
	
	const FPTWServerSettings& ServerSettings = GI->ServerSettings;
	const FString ServerType = FPTWServerSettings::ServerTypeToString(ServerSettings.ServerType);
	
	FString ErrorMessage;
	if (!GameSessionId.IsEmpty() && !ServerType.IsEmpty())
	{
		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::TerminateGameSession_Response);
		const FString APIURL = FPaths::Combine(FString(GAMELIFT_SERVERAPI_ENDPOINT), GameLift::ServerAPI::Routes::TerminateGameSession);
	
		TSharedPtr<FJsonObject> RequestJsonObject = MakeShareable(new FJsonObject());
	
		RequestJsonObject->SetStringField(TEXT("gameSessionId"), GameSessionId);
		RequestJsonObject->SetStringField(TEXT("serverType"), ServerType);
	
		FString Content;
		TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Content);
		FJsonSerializer::Serialize(RequestJsonObject.ToSharedRef(), JsonWriter);
	
		Request->SetURL(APIURL);
		Request->SetVerb(TEXT("POST"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		Request->SetContentAsString(Content);
	
		if (Request->ProcessRequest())
		{
			UE_LOG(Log_DynamoDB, Display, TEXT("[DB 레코드 제거요청] DynamoDB 레코드 제거요청 전송성공"));
			return;
		}
	}
	else
	{
		ErrorMessage = FString::Printf(
			TEXT("[데이터 누락] GameSessionId: %s, ServerType: %s"), *GameSessionId, *ServerType);
	}
	if (!ErrorMessage.IsEmpty())
	{
		ErrorMessage = TEXT(", ") + ErrorMessage;
	}
	UE_LOG(Log_DynamoDB, Error, TEXT("[DB 레코드 제거요청] DynamoDB 레코드 제거요청 전송실패%s"), *ErrorMessage);
}

void UPTWGameLiftServerSubsystem::TerminateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(Log_DynamoDB, Display, TEXT("[DB 레코드 제거응답] DynamoDB 레코드 제거성공 응답"));
	}
	else
	{
		UE_LOG(Log_DynamoDB, Error, TEXT("[DB 레코드 제거응답] DynamoDB 레코드 제거실패 응답"));
	}

	Cleanup();
	if (GameLiftSdkModule)
	{
		GameLiftSdkModule->ProcessEnding();
	}
	FGenericPlatformMisc::RequestExit(false);
}

void UPTWGameLiftServerSubsystem::UpdatePlayerCount(FString Action)
{
	UPTWGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UPTWGameInstance>() : nullptr;
	if (!IsValid(GI) || !GI->ServerSettings.IsValid()) return;
	
	FString GameSessionId = GameLiftSdkModule->GetGameSessionId().GetResult();
	
	const FPTWServerSettings& ServerSettings = GI->ServerSettings;
	const FString ServerType = FPTWServerSettings::ServerTypeToString(ServerSettings.ServerType);
	
	FString ErrorMessage;
	if (!GameSessionId.IsEmpty() && !Action.IsEmpty() && (Action == TEXT("Join") || Action == TEXT("Leave")))
	{
		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::UpdatePlayerCount_Response);
		const FString APIURL = FPaths::Combine(FString(GAMELIFT_SERVERAPI_ENDPOINT), GameLift::ServerAPI::Routes::UpdatePlayerCount);
				
		TSharedPtr<FJsonObject> RequestJsonObject = MakeShareable(new FJsonObject());
		
		// Action : Join / Leave
		RequestJsonObject->SetStringField(TEXT("gameSessionId"), GameSessionId);
		RequestJsonObject->SetStringField(TEXT("action"), Action);
		RequestJsonObject->SetStringField(TEXT("serverType"), ServerType);

		FString Content;
		TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Content);
		FJsonSerializer::Serialize(RequestJsonObject.ToSharedRef(), JsonWriter);
	
		Request->SetURL(APIURL);
		Request->SetVerb(TEXT("POST"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		Request->SetContentAsString(Content);
		
		if (Request->ProcessRequest())
		{
			UE_LOG(Log_DynamoDB, Display, TEXT("[DB 갱신요청] DynamoDB 속성 갱신요청 성공 [CurrentPlayerCount]"));
			return;
		}
	}
	else
	{
		ErrorMessage = FString::Printf(TEXT("[데이터 누락] GameSessionId: %s, Action: %s"), *GameSessionId, *Action);
	}
	if (!ErrorMessage.IsEmpty())
	{
		ErrorMessage = TEXT(", ") + ErrorMessage;
	}
	
	UE_LOG(Log_DynamoDB, Error, TEXT("[DB 갱신요청] DynamoDB 속성 갱신요청 실패 [CurrentPlayerCount]%s"), *ErrorMessage);
}

void UPTWGameLiftServerSubsystem::UpdatePlayerCount_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(Log_DynamoDB, Display, TEXT("[DB 갱신응답] DynamoDB 속성 갱신성공 응답 [CurrentPlayerCount]"));
	}
	else
	{
		UE_LOG(Log_DynamoDB, Error, TEXT("[DB 갱신응답] DynamoDB 속성 갱신실패 응답 [CurrentPlayerCount]"));
	}
}

bool UPTWGameLiftServerSubsystem::AcceptPlayerSession(const FString& UniqueId, const FString& PlayerSessionId)
{
	FGameLiftGenericOutcome Outcome = GameLiftSdkModule->AcceptPlayerSession(PlayerSessionId);
	if (Outcome.IsSuccess() && !UniqueId.IsEmpty())
	{
		PlayerSessionIds.Add(UniqueId, PlayerSessionId);
		UE_LOG(Log_GameLift, Display, TEXT("[플레이어세션 수락 허용] 플레이어 세션: %s"), *PlayerSessionId);
		return true;
	}
	else
	{
		UE_LOG(Log_GameLift, Error, TEXT("[플레이어세션 수락 거절] 플레이어 세션: %s"), *Outcome.GetError().m_errorMessage);
		return false;
	}
}

void UPTWGameLiftServerSubsystem::RemovePlayerSession(const FString& UniqueId)
{
	if (GameLiftSdkModule && PlayerSessionIds.Contains(UniqueId) && !PlayerSessionIds[UniqueId].IsEmpty())
	{
		const FString PlayerSessionId = PlayerSessionIds[UniqueId];
		PlayerSessionIds.Remove(UniqueId);
		
		GameLiftSdkModule->RemovePlayerSession(PlayerSessionId);
		UE_LOG(Log_Server, Display, TEXT("[플레이어세션 제거] 플레이어 세션: %s"), *PlayerSessionId);
	}
}

void UPTWGameLiftServerSubsystem::StopAutoBackfill()
{
	UE_LOG(Log_Server, Display, TEXT("[백필 중지] : TicketId: %s, GameSessionId: %s, MatchmakingConfiguration: %s"), 
		*MatchBackfillData.TicketId, *MatchBackfillData.GameSessionId, *MatchBackfillData.MatchmakingConfiguration);
	
	if (MatchBackfillData.IsValid())
	{
		const FStopMatchBackfillRequest StopMatchBackfillRequest(
			MatchBackfillData.TicketId, 
			MatchBackfillData.GameSessionId, 
			MatchBackfillData.MatchmakingConfiguration);
		
		GameLiftSdkModule->StopMatchBackfill(StopMatchBackfillRequest);
	}
	else
	{
		UE_LOG(Log_Server, Error, TEXT("[백필 중지실패] : 데이터 누락"));
	}
}

void UPTWGameLiftServerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPTWGameLiftServerSubsystem::Deinitialize()
{
	Cleanup();
	
	Super::Deinitialize();
}

void UPTWGameLiftServerSubsystem::Cleanup()
{
	if (FTimerManager* TimerManager = GetWorld() ? &GetWorld()->GetTimerManager() : nullptr)
	{
		if (HeartbeatTimerHandle.IsValid())
		{
			TimerManager->ClearTimer(HeartbeatTimerHandle);
			HeartbeatTimerHandle.Invalidate();
		}
	}
}
