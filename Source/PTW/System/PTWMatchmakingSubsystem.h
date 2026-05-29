#pragma once
#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PTWWebSocketClientSubsystem.h"
#include "PTWMatchmakingSubsystem.generated.h"


namespace MatchmakingText
{
	extern const FText Initialized;
	extern const FText Queued;
	extern const FText Searching;
	extern const FText Searching_Retry;
	extern const FText Match_Found;
	extern const FText Requires_Acceptance;
	extern const FText Placing;
	extern const FText Placing_Accepted;
	extern const FText Placing_Rejected;
	extern const FText Placing_Timeout;
	extern const FText Completed;
	extern const FText Timed_Out;
	extern const FText Cancelled;
	extern const FText Cancelled_Rejected;
	extern const FText Cancelled_Accept_Timeout;
	extern const FText Failed;
}

/*
 * 매치메이킹 플로우 상태를 나타내는 열거형입니다.
 * 현재 진행하는 매치메이킹 상태/단계를 나타냅니다.
 * [Matchmaking Ticket의 Status를 참고하여 작성했습니다.]
 */
UENUM(BlueprintType)
enum class EPTWMatchmakingFlow : uint8
{
	// 매치메이킹 초기화 상태.
	NONE							UMETA(DisplayName = "NONE"),
	
	// QUEUED 매치메이킹 요청이 접수된 상태.
	QUEUED							UMETA(DisplayName = "QUEUED"),
	
	// 매치메이킹 구성을 참고로 매칭 대상을 탐색중인 상태. (MatchmakingSearching)
	SEARCHING						UMETA(DisplayName = "SEARCHING"),
	
	// 매칭인원 수락단계 없이 유효한 매치를 찾은 상태. (PotentialMatchCreated)
	MATCH_FOUND						UMETA(DisplayName = "MATCH_FOUND"),
	
	// 최소 매칭인원을 만족하여 매칭수락을 대기/진행하는 상태. (PotentialMatchCreated) or [ACCEPTING]
	REQUIRES_ACCEPTANCE				UMETA(DisplayName = "REQUIRES_ACCEPTANCE"),
	
	// 매칭인원 모두가 수락을 완료하여 서버를 배치중인 상태. (AcceptMatchCompleted)
	PLACING							UMETA(DisplayName = "PLACING"),
	
	// 서버가 성공적으로 생성되어 서버로 진입하는 상태. (MatchmakingSucceeded)
	COMPLETED						UMETA(DisplayName = "COMPLETED"),
	
	// 최대 매칭가능 시간을 넘어 타임아웃이 발생한 상태. (MatchmakingTimedOut)
	TIMED_OUT						UMETA(DisplayName = "TIMED_OUT"),
	
	// 기타 이유로 매칭이 취소된 상태. (MatchmakingCancelled)
	CANCELLED						UMETA(DisplayName = "CANCELLED"),
	
	// 시스템 오류로 인한 매칭 실패 상태. (MatchmakingFailed)
	FAILED							UMETA(DisplayName = "FAILED"),
	
	// 매치메이킹 종료 상태.
	END								UMETA(DisplayName = "END")
};

/*
 * 매치메이킹 플로우의 세부 사유를 나타내는 열거형입니다.
 * 홀로 사용되지 않고, 매치메이킹 플로우와 함께 사용됩니다.
 */
UENUM(BlueprintType)
enum class EPTWMatchmakingReason : uint8
{
	// 초기화
	NONE							UMETA(DisplayName = "NONE"),
	
	// [SEARCHING] 기타 이유로 매칭 대상을 다시 탐색중인 상태. (MatchmakingSearching - Retry)
	SEARCHING_RETRY					UMETA(DisplayName = "SEARCHING_RETRY"),
	
	// [REQUIRES_ACCEPTANCE] 누군가의 매칭 수락 요청이 전송된 상태. (AcceptMatch)
	REQUIRES_ACCEPTANCE_ACCEPTING	UMETA(DisplayName = "REQUIRES_ACCEPTANCE_ACCEPTING"),

	// [PLACING] 매칭 정상 수락으로 인한 매칭 수락 단계가 종료된 상태. (PLACING - Accepted)
	PLACING_ACCEPTED				UMETA(DisplayName = "PLACING_ACCEPTED"),
	
	// [PLACING] 매칭 수락 거절로 인한 매칭 수락 단계가 종료된 상태. (PLACING - Rejected)
	PLACING_REJECTED				UMETA(DisplayName = "PLACING_REJECTED"),
	
	// [PLACING] 매칭 타임아웃으로 인한 매칭 수락 단계가 종료된 상태. (PLACING - TimedOut)
	PLACING_TIMEOUT					UMETA(DisplayName = "PLACING_TIMEOUT"),
	
	// [CANCELLED] 매칭 수락을 거절하여 매칭이 취소된 상태. (MatchmakingCancelled - AcceptanceTimeout)
	CANCELLED_REJECTED				UMETA(DisplayName = "CANCELLED_REJECTED"),
	
	// [CANCELLED] 매칭 수락 단계에서 타임아웃이 발생해 매칭이 취소된 상태. (MatchmakingCancelled - MatchRejected)
	CANCELLED_ACCEPT_TIMEOUT		UMETA(DisplayName = "CANCELLED_ACCEPT_TIMEOUT"),
	
	// 종료
	END								UMETA(DisplayName = "END")
};

USTRUCT(BlueprintType)
struct FPTWMatchmakingState
{
	GENERATED_BODY()
	
	UPROPERTY() FString TicketId = FString();
	UPROPERTY() EPTWMatchmakingFlow Flow = EPTWMatchmakingFlow::NONE;
	UPROPERTY() EPTWMatchmakingReason Reason = EPTWMatchmakingReason::NONE;
	UPROPERTY() bool bIsBackFill = false;	// BackFill로 참여 여부를 나타내는 Bool
	UPROPERTY() int32 TotalPlayers = INDEX_NONE;
	UPROPERTY() int32 AcceptedPlayers = INDEX_NONE;
	UPROPERTY() int32 EventTime = INDEX_NONE;
	UPROPERTY() int32 AcceptanceTimeout = INDEX_NONE;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchmakingFlowChanged);

/**
 * 게임 매치메이킹을 관리하는 LocalPlayerSubsystem.
 */
UCLASS()
class PTW_API UPTWMatchmakingSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	UPTWMatchmakingSubsystem();
	static UPTWMatchmakingSubsystem* Get(const UObject* WorldContextObject);
	FORCEINLINE const FPTWMatchmakingState& GetCurrentState() const { return CurrentState; }
	
	void InitMatchmaking(bool bNotify = false);
	void OnReceivePayload(const FString& Payload);
	void ProcessJitterBuffer();
	void HandleReceivePayload(const FString& Payload);
	void ShowWidget();
	void HideWidget();
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	EPTWMatchmakingFlow ProgressMatchmakingFlow(const FString& EventType);
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> ServerConnectWidgetClass;
	
	UPROPERTY(VisibleAnywhere)
	FPTWMatchmakingState CurrentState;
	
public:
	FOnMatchmakingFlowChanged OnMatchmakingFlowChanged;
	
	UPROPERTY()
	FPTWNetworkEvent MatchmakingEvent; 
	
protected:
	FTimerHandle ProcessTimer;
	
};
