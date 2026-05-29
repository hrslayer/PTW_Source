#include "PTWMatchmakingSubsystem.h"
#include "PTWGameLiftClientSubsystem.h"
#include "Utilities/PTWJsonUtility.h"
#include "UI/PTWUISubsystem.h"
#include "UI/MainMenu/PTWServerConnectWidget.h"


#define LOCTEXT_NAMESPACE "Matchmaking"
namespace MatchmakingText
{
	const FText Initialized					= LOCTEXT("Initialized", "[매치메이킹]\n매치메이킹 준비 중");
	const FText Queued						= LOCTEXT("Queued", "[매치메이킹]\n매칭 대기열 등록 중");
	const FText Searching					= LOCTEXT("Searching", "[매치메이킹]\n유효한 매치 탐색 중");
	const FText Searching_Retry				= LOCTEXT("Searching_Retry", "[매치메이킹]\n유효한 매치 재탐색 중");
	const FText Match_Found					= LOCTEXT("Match_Found", "[매치메이킹]\n유효한 매치를 찾았습니다.");
	const FText Requires_Acceptance			= LOCTEXT("Requires_Acceptance", "[매치메이킹]\n준비가 완료 되었다면 수락을 눌러주세요.");
	const FText Placing						= LOCTEXT("Placing", "[매치메이킹]\n매칭 수락 단계가 종료되었습니다.\n[사유: 알 수 없음]");
	const FText Placing_Accepted			= LOCTEXT("Placing_Accepted", "[매치메이킹]\n모든 플레이어가 매칭을 수락했습니다.\n[서버 배치 중]");
	const FText Placing_Rejected			= LOCTEXT("Placing_Rejected", "[매치메이킹]\n매칭 수락 단계가 종료되었습니다.\n[매칭 수락 거절]");
	const FText Placing_Timeout				= LOCTEXT("Placing_Timeout", "[매치메이킹]\n매칭 수락 단계가 종료되었습니다.\n[매칭 수락 만료]");
	const FText Completed					= LOCTEXT("Completed", "[매치메이킹]\n서버 생성 완료\n서버 접속 중");
	const FText Timed_Out					= LOCTEXT("Timed_Out", "[매치메이킹]\n유효한 매치를 찾지 못했습니다.");
	const FText Cancelled					= LOCTEXT("Cancelled", "[매치메이킹]\n매칭이 취소되었습니다.");
	const FText Cancelled_Rejected			= LOCTEXT("Cancelled_Rejected", "[매치메이킹]\n매칭이 취소되었습니다.\n[매칭 수락 거절]");
	const FText Cancelled_Accept_Timeout	= LOCTEXT("Cancelled_Accept_Timeout", "[매치메이킹]\n매칭이 취소되었습니다.\n[매칭 수락 만료]");
	const FText Failed						= LOCTEXT("Failed", "[매치메이킹]\n매칭시스템에 오류가 발생했습니다.");
}
#undef LOCTEXT_NAMESPACE

UPTWMatchmakingSubsystem::UPTWMatchmakingSubsystem()
{
	static ConstructorHelpers::FClassFinder<UPTWServerConnectWidget> WidgetClassFinder(TEXT("/Game/_PTW/UI/MainMenu/WBP_PTWServerConnectWidget"));
	if (WidgetClassFinder.Succeeded())
	{
		ServerConnectWidgetClass = WidgetClassFinder.Class; 
	}
}

UPTWMatchmakingSubsystem* UPTWMatchmakingSubsystem::Get(const UObject* WorldContextObject)
{
	if (GEngine)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
				{
					return LocalPlayer->GetSubsystem<UPTWMatchmakingSubsystem>();
				}
			}
		}
	}
	return nullptr;
}

void UPTWMatchmakingSubsystem::InitMatchmaking(bool bNotify)
{
	// EPTWMatchmakingFlow::NONE
	CurrentState = FPTWMatchmakingState();
	if (bNotify)
	{
		OnMatchmakingFlowChanged.Broadcast();
	}
}

void UPTWMatchmakingSubsystem::OnReceivePayload(const FString& Payload)
{
	if (!GetWorld()) return;
	const FString STARTTIME = TEXT("StartTime");
	const FString EVENTTIME = TEXT("EventTime");
	TArray<FString> FieldsToExtract = { STARTTIME, EVENTTIME };
	TMap<FString, FString> ParsedData = UPTWJsonUtility::ExtractFieldsFromHTTPData(Payload, FieldsToExtract);
	if (!ParsedData.Contains(STARTTIME) || !ParsedData.Contains(EVENTTIME) || 
		ParsedData[STARTTIME].IsEmpty() || ParsedData[EVENTTIME].IsEmpty())
	{
		const FString StartTime = ParsedData.Contains(STARTTIME) ? ParsedData[STARTTIME] : TEXT("");
		const FString EventTime = ParsedData.Contains(EVENTTIME) ? ParsedData[EVENTTIME] : TEXT("");
		UE_LOG(LogTemp, Display, TEXT("[데이터 누락] Payload가 유효하지 않습니다. StartTime: %s, EventTime: %s"), 
			*StartTime, *EventTime)
		return;
	}
	const int64 StartTime = FCString::Atoi64(*ParsedData[STARTTIME]);
	const int64 EventTime = FCString::Atoi64(*ParsedData[EVENTTIME]);
	
	if (MatchmakingEvent.StartTime > StartTime) return;
	if (MatchmakingEvent.LatestEventTime > EventTime) return;
	
	const FPTWNetworkPacket NetworkPacket = FPTWNetworkPacket(EventTime, Payload);
	if (!MatchmakingEvent.PendingPackets.Contains(NetworkPacket))
	{
		MatchmakingEvent.PendingPackets.Push(NetworkPacket);
	}
	
	FTimerManager* TimerManager = GetWorld() ? &GetWorld()->GetTimerManager() : nullptr;
	if (!TimerManager->IsTimerActive(ProcessTimer))
	{
		TimerManager->SetTimer(ProcessTimer, this, &ThisClass::ProcessJitterBuffer, 0.2f, false);
	}
}

void UPTWMatchmakingSubsystem::ProcessJitterBuffer()
{
	MatchmakingEvent.ActivePackets.Swap(MatchmakingEvent.PendingPackets);
	while (MatchmakingEvent.ActivePackets.Num() > 0)
	{
		const FPTWNetworkPacket NetworkPacket = MatchmakingEvent.ActivePackets.Pop();
		HandleReceivePayload(NetworkPacket.Payload);
		MatchmakingEvent.LatestEventTime = NetworkPacket.Timestamp > MatchmakingEvent.LatestEventTime ? NetworkPacket.Timestamp : MatchmakingEvent.LatestEventTime;
	}
}

void UPTWMatchmakingSubsystem::HandleReceivePayload(const FString& Payload)
{
	const FString TICKETID = TEXT("TicketId");
	const FString EVENTTYPE = TEXT("EventType");
	const FString EVENTTIME = TEXT("EventTime");
	const FString EVENTREASON = TEXT("EventReason");
	
	TMap<FString, FString> BasicParsedData;
	{
		TArray<FString> FieldsToExtract = { EVENTTYPE, TICKETID, EVENTTIME, EVENTREASON };
		BasicParsedData = UPTWJsonUtility::ExtractFieldsFromHTTPData(Payload, FieldsToExtract);
	}
	if (!(BasicParsedData.Contains(EVENTTYPE) && BasicParsedData.Contains(TICKETID) && 
		BasicParsedData.Contains(EVENTTIME) && BasicParsedData.Contains(EVENTREASON))) return;
	
	EPTWMatchmakingFlow PrevFlow = CurrentState.Flow;
	const FString Reason = BasicParsedData[EVENTREASON];
	CurrentState.Flow = ProgressMatchmakingFlow(BasicParsedData[EVENTTYPE]);
	CurrentState.TicketId = BasicParsedData[TICKETID];
	
	if (PrevFlow == CurrentState.Flow && (CurrentState.Flow != EPTWMatchmakingFlow::REQUIRES_ACCEPTANCE)) return;
	
	UPTWGameLiftClientSubsystem* GameLiftClientSubsystem = UPTWGameLiftClientSubsystem::Get(this);
	if (!IsValid(GameLiftClientSubsystem)) return;
	
	switch (CurrentState.Flow)
	{
	case EPTWMatchmakingFlow::QUEUED:
		CurrentState.Reason = EPTWMatchmakingReason::NONE;
		OnMatchmakingFlowChanged.Broadcast();
		break;
	case EPTWMatchmakingFlow::SEARCHING:
		CurrentState.Reason = EPTWMatchmakingFlow::SEARCHING < PrevFlow ? EPTWMatchmakingReason::SEARCHING_RETRY : EPTWMatchmakingReason::NONE;
		OnMatchmakingFlowChanged.Broadcast();
		break;
	case EPTWMatchmakingFlow::MATCH_FOUND:
		CurrentState.Reason = EPTWMatchmakingReason::NONE;
		OnMatchmakingFlowChanged.Broadcast();
		break;
	case EPTWMatchmakingFlow::REQUIRES_ACCEPTANCE:
		{
			if (CurrentState.TotalPlayers == INDEX_NONE)
			{
				const FString ACCEPTANCETIMEOUT = TEXT("AcceptanceTimeout");
				const FString TOTALPLAYERS = TEXT("TotalPlayers");
				const FString ACCEPTEDPLAYERS = TEXT("AcceptedPlayers");
				TArray<FString> FieldsToExtract = { ACCEPTANCETIMEOUT, TOTALPLAYERS, ACCEPTEDPLAYERS };
				
				TMap<FString, FString> ParsedData = BasicParsedData = UPTWJsonUtility::ExtractFieldsFromHTTPData(Payload, FieldsToExtract);
				if (!ParsedData.Contains(ACCEPTANCETIMEOUT) || !ParsedData.Contains(TOTALPLAYERS) || !ParsedData.Contains(ACCEPTEDPLAYERS)) return;
		
				CurrentState.AcceptanceTimeout = FCString::Atoi(*ParsedData[ACCEPTANCETIMEOUT]);
				CurrentState.TotalPlayers = FCString::Atoi(*ParsedData[TOTALPLAYERS]);
				CurrentState.AcceptedPlayers = FCString::Atoi(*ParsedData[ACCEPTEDPLAYERS]);
			}
			CurrentState.Reason = Reason == "Accepting" ? EPTWMatchmakingReason::REQUIRES_ACCEPTANCE_ACCEPTING : EPTWMatchmakingReason::NONE;
		}
		OnMatchmakingFlowChanged.Broadcast();
		break;
	case EPTWMatchmakingFlow::PLACING:
		if (Reason == "Accepted")
		{
			CurrentState.Reason = EPTWMatchmakingReason::PLACING_ACCEPTED;
		}
		else if (Reason == "Rejected")
		{
			CurrentState.Reason = EPTWMatchmakingReason::PLACING_REJECTED;
		}
		else if (Reason == "TimeOut")
		{
			CurrentState.Reason = EPTWMatchmakingReason::PLACING_TIMEOUT;
		}
		else
		{
			CurrentState.Reason = EPTWMatchmakingReason::NONE;
		}
		OnMatchmakingFlowChanged.Broadcast();
		break;
	case EPTWMatchmakingFlow::COMPLETED:
		CurrentState.Reason = EPTWMatchmakingReason::NONE;
		OnMatchmakingFlowChanged.Broadcast();
		{
			const FString STEAMID = TEXT("SteamId");
			const FString PLAYERSESSIONID = TEXT("PlayerSessionId");
			TArray<FString> FieldsToExtract = { STEAMID, PLAYERSESSIONID };
			TMap<FString, FString> ParsedData = UPTWJsonUtility::ExtractFieldsFromHTTPData(Payload, FieldsToExtract);
			
			if (ParsedData.Contains(STEAMID) && ParsedData.Contains(PLAYERSESSIONID))
			{
				const FString SteamId = ParsedData[STEAMID];
				const FString PlayerSessionId = ParsedData[PLAYERSESSIONID];
				GameLiftClientSubsystem->JoinGameSession(SteamId, PlayerSessionId);
			}
		}
		InitMatchmaking();
		break;
	case EPTWMatchmakingFlow::TIMED_OUT:
		CurrentState.Reason = EPTWMatchmakingReason::NONE;
		OnMatchmakingFlowChanged.Broadcast();
		break;
	case EPTWMatchmakingFlow::CANCELLED:
		if (Reason == "MatchRejected")
		{
			CurrentState.Reason = EPTWMatchmakingReason::CANCELLED_REJECTED;
		}
		else if (Reason == "AcceptanceTimeout")
		{
			CurrentState.Reason = EPTWMatchmakingReason::CANCELLED_ACCEPT_TIMEOUT;
		}
		else
		{
			CurrentState.Reason = EPTWMatchmakingReason::NONE;
		}
		OnMatchmakingFlowChanged.Broadcast();
		break;
	case EPTWMatchmakingFlow::FAILED:
		CurrentState.Reason = EPTWMatchmakingReason::NONE;
		OnMatchmakingFlowChanged.Broadcast();
		break;
	default:
		break;
	}
}

void UPTWMatchmakingSubsystem::ShowWidget()
{
	UPTWUISubsystem* UISubsystem = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController())
		{
			UISubsystem = LocalPlayer->GetSubsystem<UPTWUISubsystem>();
		}
	}
	if (!IsValid(UISubsystem)) return;
	
	UISubsystem->ShowSystemWidget(ServerConnectWidgetClass);
}

void UPTWMatchmakingSubsystem::HideWidget()
{
	UPTWUISubsystem* UISubsystem = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController())
		{
			UISubsystem = LocalPlayer->GetSubsystem<UPTWUISubsystem>();
		}
	}
	if (!IsValid(UISubsystem)) return;
	
	UISubsystem->HideSystemWidget(ServerConnectWidgetClass);
}

void UPTWMatchmakingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPTWMatchmakingSubsystem::Deinitialize()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ProcessTimer);
	}
	Super::Deinitialize();
}

EPTWMatchmakingFlow UPTWMatchmakingSubsystem::ProgressMatchmakingFlow(const FString& EventType)
{
	// 열거형을 문자열로 변환 및 탐색 결과(위치)를 반환.
	int64 EnumValue = StaticEnum<EPTWMatchmakingFlow>()->GetValueByNameString(EventType);

	// 해당하는 열거형을 찾지 못한 경우.
	if (EnumValue == INDEX_NONE)
	{
		return EPTWMatchmakingFlow::NONE;
	}

	// 해당하는 열거형을 찾아 대입.
	EPTWMatchmakingFlow NewFlow = static_cast<EPTWMatchmakingFlow>(EnumValue);

	// 찾은 열거형이 초기값인 경우
	if (NewFlow == EPTWMatchmakingFlow::NONE || NewFlow == EPTWMatchmakingFlow::END)
	{
		return NewFlow;
	}
	
	// Search 상태는 무조건 적용
	if (NewFlow == EPTWMatchmakingFlow::SEARCHING && CurrentState.Flow == EPTWMatchmakingFlow::CANCELLED &&
		CurrentState.Reason != EPTWMatchmakingReason::NONE)
	{
		return NewFlow;
	}
	
	// 현재 플로우가 순서상 마지막 플로우 (COMPLETED) 인가?
	if (CurrentState.Flow >= EPTWMatchmakingFlow::COMPLETED)
	{
		return CurrentState.Flow;
	}
	
	// 다음 단계 플로우가 들어온 경우
	if (CurrentState.Flow <= NewFlow)
	{
		return NewFlow;
	}
	
	// 이전 플로우가 입력된 경우, 역전현상을 방지.
	return CurrentState.Flow;
}
