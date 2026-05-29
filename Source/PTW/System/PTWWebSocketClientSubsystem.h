#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IWebSocket.h"
#include "System/Utilities/PTWContainers.h"
#include "PTWWebSocketClientSubsystem.generated.h"


USTRUCT(BlueprintType)
struct FPTWNetworkPacket
{
	GENERATED_BODY()
	
	UPROPERTY() int64 Timestamp;
	UPROPERTY() FString Payload;

	bool operator==(const FPTWNetworkPacket& Other) const
	{
		return Timestamp == Other.Timestamp && Payload == Other.Payload;
	}
	bool operator<(const FPTWNetworkPacket& Other) const
	{
		return Timestamp < Other.Timestamp;
	}
};

USTRUCT(BlueprintType)
struct FPTWNetworkEvent
{
	GENERATED_BODY()
	
	TPriorityQueue<FPTWNetworkPacket> PendingPackets;
	TPriorityQueue<FPTWNetworkPacket> ActivePackets;
	int64 LatestEventTime = 0;
	int64 StartTime = 0;
};

class IWebSocket;
class UPTWServerConnectWidget;

/**
 * AWS API Gateway에서 관리되는 클라이언트 전용 WebSocket 서브시스템입니다.
 */
UCLASS()
class PTW_API UPTWWebSocketClientSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UPTWWebSocketClientSubsystem();
	static UPTWWebSocketClientSubsystem* Get(const UObject* WorldContextObject);
	
	FORCEINLINE bool IsWebSocketValid() const { return WebSocket.IsValid(); }
	FORCEINLINE bool IsWebSocketConnected() const { return WebSocket->IsConnected(); }
	
	FString GetActionValue(const FString& JsonData) const;

	UFUNCTION(BlueprintCallable, Category = "Network|WebSocket")
	void ConnectToWebSocket(const FString& LocalUniqueId);
	
	UFUNCTION(BlueprintCallable, Category = "Network|WebSocket")
	void DisconnectToWebSocket();
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void OnConnected();
	void OnConnectionError(const FString& Error);
	void OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
	void OnMessageReceived(const FString& Message);
	
	void HandleClosed();
	void SendHeartbeat();
	
public:
	TSharedPtr<IWebSocket> WebSocket;
	
protected:
	FTimerHandle HeartbeatTimerHandle;
	
};
