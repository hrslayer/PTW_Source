#include "PTWWebSocketClientSubsystem.h"
#include "PTWMatchmakingSubsystem.h"
#include "WebSocketsModule.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "Utilities/PTWJsonUtility.h"
#include "API/PTWAPIData.h"
#include "Debug/PTWLogCategorys.h"

UPTWWebSocketClientSubsystem::UPTWWebSocketClientSubsystem()
{
}

UPTWWebSocketClientSubsystem* UPTWWebSocketClientSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (IsValid(World))
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UPTWWebSocketClientSubsystem>();
		}
	}
	return nullptr;
}

FString UPTWWebSocketClientSubsystem::GetActionValue(const FString& JsonData) const
{
	FString ActionName = TEXT("Action");
	TArray<FString> FieldsToExtract = { ActionName };
	TMap<FString, FString> SessionData = UPTWJsonUtility::ExtractFieldsFromHTTPData(JsonData, FieldsToExtract);
	if (SessionData.Contains(ActionName))
	{
		return SessionData[ActionName];
	}
	return FString();
}

void UPTWWebSocketClientSubsystem::ConnectToWebSocket(const FString& LocalUniqueId)
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		WebSocket->Close();
	}
	if (LocalUniqueId.IsEmpty()) return;
	
	const FString BaseURL = FString(WEBSOCKET_CLIENTAPI_ENDPOINT);
	const FString AccountId = LocalUniqueId;
	
	FString APIURL = FString::Printf(TEXT("%s?accountId=%s"), *BaseURL, *AccountId);
	FString ServerProtocol = TEXT("wss");
	
	WebSocket = FWebSocketsModule::Get().CreateWebSocket(APIURL, ServerProtocol);
	
	WebSocket->OnConnected().AddUObject(this, &ThisClass::OnConnected);
	WebSocket->OnConnectionError().AddUObject(this, &ThisClass::OnConnectionError);
	WebSocket->OnClosed().AddUObject(this, &ThisClass::OnClosed);
	WebSocket->OnMessage().AddUObject(this, &ThisClass::OnMessageReceived);
	
	WebSocket->Connect();
	UE_LOG(Log_WebSocket, Log, TEXT("[웹소켓 연결요청] 웹소켓 연결 요청"));
}

void UPTWWebSocketClientSubsystem::DisconnectToWebSocket()
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		WebSocket->Close(1000);
	}
}

void UPTWWebSocketClientSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (IsRunningDedicatedServer() || UE_EDITOR) return;
	
	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
	}
	
	if (UPTWGameInstance* GI = GetWorld()->GetGameInstance<UPTWGameInstance>())
	{
		if (GI->LocalUniqueId.IsEmpty())
		{
			GI->OnLocalUniqueIdCreated.AddUniqueDynamic(this, &ThisClass::ConnectToWebSocket);
		}
		else
		{
			ConnectToWebSocket(GI->LocalUniqueId);
		}
	}
}

void UPTWWebSocketClientSubsystem::Deinitialize()
{
	if (UPTWGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UPTWGameInstance>() : nullptr)
	{
		GI->OnLocalUniqueIdCreated.RemoveDynamic(this, &ThisClass::ConnectToWebSocket);
	}
	DisconnectToWebSocket();
	
	Super::Deinitialize();
}

void UPTWWebSocketClientSubsystem::OnConnected()
{
	UE_LOG(Log_WebSocket, Log, TEXT("[웹소켓 연결응답] 웹소켓 연결 성공"));
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			HeartbeatTimerHandle,
			this,
			&ThisClass::SendHeartbeat,
			300.0f,
			true
			);
	}
}

void UPTWWebSocketClientSubsystem::OnConnectionError(const FString& Error)
{
	UE_LOG(Log_WebSocket, Error, TEXT("[웹소켓 연결오류] 웹소켓 연결 오류: %s"), *Error);
	HandleClosed();
}

void UPTWWebSocketClientSubsystem::OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	UE_LOG(Log_WebSocket, Log, TEXT("[웹소켓 연결해제] 웹소켓 연결 해제됨. Reason: %s"), *Reason);
	HandleClosed();
}

void UPTWWebSocketClientSubsystem::OnMessageReceived(const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("AWS로부터 메세지 수신: %s"), *Message);

	FString Action = GetActionValue(Message);
	
	if (Action == WebSocket::ClientAPI::Routes::Receive::Pong)
	{
		UE_LOG(Log_WebSocket, Log, TEXT("[웹소켓 하트비트] 웹소켓 하트비트 수신성공"));
	}
	else if (Action == WebSocket::ClientAPI::Routes::Receive::Matchmaking)
	{
		if (UPTWMatchmakingSubsystem* MatchmakingSubsystem = UPTWMatchmakingSubsystem::Get(this))
		{
			MatchmakingSubsystem->OnReceivePayload(Message);
		}
	}
}

void UPTWWebSocketClientSubsystem::HandleClosed()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
	}
}

void UPTWWebSocketClientSubsystem::SendHeartbeat()
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		TSharedPtr<FJsonObject> RequestJsonObject = MakeShareable(new FJsonObject());
		RequestJsonObject->SetStringField(TEXT("action"), WebSocket::ClientAPI::Routes::Send::Ping);
		FString Content;
		TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Content);
		FJsonSerializer::Serialize(RequestJsonObject.ToSharedRef(), JsonWriter);
		
		WebSocket->Send(Content);
		UE_LOG(Log_WebSocket, Log, TEXT("[웹소켓 하트비트] 웹소켓 하트비트 전송성공"));
	}
}
