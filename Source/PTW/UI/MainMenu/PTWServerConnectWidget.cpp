#include "PTWServerConnectWidget.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/WidgetSwitcher.h"
#include "System/PTWGameLiftClientSubsystem.h"
#include "System/PTWMatchmakingSubsystem.h"

void UPTWServerConnectWidget::HandleStartTimer()
{
	StartTimer++;
	if (!IsValid(StartTimerText) || !bShowTimerTick) return;
	
	int32 Minute = StartTimer / 60;
	int32 Second = StartTimer % 60;
	
	FString RefinedSec = Second < 10 ? TEXT("0") + FString::FromInt(Second) : FString::FromInt(Second);
	FString TimeText = FString::FromInt(Minute) + ":" + RefinedSec;
	
	StartTimerText->SetText(FText::FromString(TimeText));
}

void UPTWServerConnectWidget::HandleAcceptTimer()
{
	if (AcceptTimer <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(AcceptanceTimeoutHandle);
		return;
	}
	AcceptTimer--;
	if (!IsValid(AcceptTimerText) || !bShowTimerTick) return;
	
	int32 Minuate = AcceptTimer / 60;
	int32 Seccond = AcceptTimer % 60;
	
	FString RefinedSec = Seccond < 10 ? TEXT("0") + FString::FromInt(Seccond) : FString::FromInt(Seccond);
	FString TimeText = FString::FromInt(Minuate) + ":" + RefinedSec;
	
	AcceptTimerText->SetText(FText::FromString(TimeText));
}

void UPTWServerConnectWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (UPTWMatchmakingSubsystem* MatchmakingSubsystem = UPTWMatchmakingSubsystem::Get(this))
	{
		MatchmakingSubsystem->OnMatchmakingFlowChanged.AddUniqueDynamic(this, &ThisClass::HandleMatchmakingFlowChanged);
	}
	
	AcceptButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClickedAccept);
	CancelButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClickedCancel);
	RejectButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClickedReject);
}

void UPTWServerConnectWidget::NativeDestruct()
{
	if (UPTWMatchmakingSubsystem* MatchmakingSubsystem = UPTWMatchmakingSubsystem::Get(this))
	{
		MatchmakingSubsystem->OnMatchmakingFlowChanged.RemoveDynamic(this, &ThisClass::HandleMatchmakingFlowChanged);
	}
	GetWorld()->GetTimerManager().ClearTimer(StartTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(AcceptanceTimeoutHandle);
	
	AcceptButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClickedAccept);
	CancelButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClickedCancel);
	RejectButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClickedReject);
	
	Super::NativeDestruct();
}

void UPTWServerConnectWidget::HandleMatchmakingFlowChanged()
{
	UPTWMatchmakingSubsystem* MatchmakingSubsystem = UPTWMatchmakingSubsystem::Get(this);
	if (!IsValid(MatchmakingSubsystem)) return;
	
	const FPTWMatchmakingState& CurrentState = MatchmakingSubsystem->GetCurrentState();
	
	if (NoButtonSwitcher->GetActiveWidgetIndex() != CANCEL_BUTTON_INDEX && CurrentState.Flow != EPTWMatchmakingFlow::REQUIRES_ACCEPTANCE)
	{
		NoButtonSwitcher->SetActiveWidgetIndex(CANCEL_BUTTON_INDEX);
	}
	if (CurrentState.Flow != EPTWMatchmakingFlow::REQUIRES_ACCEPTANCE && CurrentState.Flow != EPTWMatchmakingFlow::PLACING)
	{
		if (AcceptanceHorizontal->GetChildrenCount() > 0)
		{
			AcceptanceHorizontal->ClearChildren();
		}
	}
	
	switch (CurrentState.Flow)
	{
	case EPTWMatchmakingFlow::NONE:
		HandleFlowNone();
		break;
	case EPTWMatchmakingFlow::QUEUED:
		HandleFlowQueued();
		break;
	case EPTWMatchmakingFlow::SEARCHING:
		HandleFlowSearching();
		break;
	case EPTWMatchmakingFlow::MATCH_FOUND:
		HandleFlowMatchFound();
		break;
	case EPTWMatchmakingFlow::REQUIRES_ACCEPTANCE:
		HandleFlowRequiresAcceptance();
		break;
	case EPTWMatchmakingFlow::PLACING:
		HandleFlowPlacing();
		break;
	case EPTWMatchmakingFlow::COMPLETED:
		HandleFlowCompleted();
		break;
	case EPTWMatchmakingFlow::TIMED_OUT:
		HandleFlowTimedOut();
		break;
	case EPTWMatchmakingFlow::CANCELLED:
		HandleFlowCancelled();
		break;
	case EPTWMatchmakingFlow::FAILED:
		HandleFlowFailed();
		break;
	default:
		break;
	}
}

void UPTWServerConnectWidget::UpdateAcceptanceProgress(int32 TotalPlayers, int32 AcceptedPlayers)
{
	int32 CurrentCount = TotalPlayers;
	while (CurrentCount != AcceptanceHorizontal->GetChildrenCount())
	{
		// 위젯이 부족할 때
		if (CurrentCount < AcceptanceHorizontal->GetChildrenCount())
		{
			AcceptanceHorizontal->RemoveChildAt(0);
		}
		else if (CurrentCount > AcceptanceHorizontal->GetChildrenCount())
		{
			UImage* PendingImage = NewObject<UImage>();
			PendingImage->SetBrushFromTexture(PendingTexture, true);
			
			UHorizontalBoxSlot* HorizontalBoxSlot = AcceptanceHorizontal->AddChildToHorizontalBox(PendingImage);
			HorizontalBoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			HorizontalBoxSlot->SetHorizontalAlignment(HAlign_Center);
			HorizontalBoxSlot->SetVerticalAlignment(VAlign_Center);
		}
	}
	
	for (int32 Index = 0; Index < AcceptanceHorizontal->GetChildrenCount(); Index++)
	{
		UWidget* Widget = AcceptanceHorizontal->GetChildAt(Index);
		if (!IsValid(Widget)) continue;
		
		UImage* Image = Cast<UImage>(Widget);
		if (!IsValid(Image)) continue;
		
		UObject* ResourceObject = Image->GetBrush().GetResourceObject();
		if (!IsValid(ResourceObject)) continue;
			
		UTexture2D* ExtractedTexture = Cast<UTexture2D>(ResourceObject);
		if (!IsValid(ExtractedTexture)) continue;
		
		if (Index < AcceptedPlayers && ExtractedTexture == PendingTexture)
		{
			Image->SetBrushFromTexture(AcceptedTexture, true);
		}
		else if (Index >= AcceptedPlayers && ExtractedTexture == AcceptedTexture)
		{
			Image->SetBrushFromTexture(PendingTexture, true);
		}
	}
}

void UPTWServerConnectWidget::HandleFlowNone()
{
	TimerSwitcher->SetActiveWidgetIndex(TIMER_START_INDEX);
	bShowTimerTick = true;
	CancelButton->SetIsEnabled(false);
		
	MessageText->SetText(MatchmakingText::Initialized);
	StartTimerText->SetText(FText::FromString(TEXT("0:00")));
		
	StartTimer = 0;
	GetWorld()->GetTimerManager().SetTimer(StartTimerHandle, this, &ThisClass::HandleStartTimer, 1.0f, true);
	AcceptButton->SetIsEnabled(false);
}

void UPTWServerConnectWidget::HandleFlowQueued()
{
	MessageText->SetText(MatchmakingText::Queued);
	bShowTimerTick = true;
	CancelButton->SetIsEnabled(false);
}

void UPTWServerConnectWidget::HandleFlowSearching()
{
	UPTWMatchmakingSubsystem* MatchmakingSubsystem = UPTWMatchmakingSubsystem::Get(this);
	if (!IsValid(MatchmakingSubsystem)) return;
	
	const FPTWMatchmakingState& CurrentState = MatchmakingSubsystem->GetCurrentState();
	
	if (CurrentState.Reason != EPTWMatchmakingReason::SEARCHING_RETRY)
	{
		MessageText->SetText(MatchmakingText::Searching);
	}
	else
	{
		MessageText->SetText(MatchmakingText::Searching_Retry);
	}
	bShowTimerTick = true;
	CancelButton->SetIsEnabled(true);
}

void UPTWServerConnectWidget::HandleFlowMatchFound()
{
	MessageText->SetText(MatchmakingText::Match_Found);
	bShowTimerTick = true;
	CancelButton->SetIsEnabled(true);
}

void UPTWServerConnectWidget::HandleFlowRequiresAcceptance()
{
	UPTWMatchmakingSubsystem* MatchmakingSubsystem = UPTWMatchmakingSubsystem::Get(this);
	if (!IsValid(MatchmakingSubsystem)) return;
	
	const FPTWMatchmakingState& CurrentState = MatchmakingSubsystem->GetCurrentState();
	
	if (TimerSwitcher->GetActiveWidgetIndex() != TIMER_ACCEPT_INDEX)
	{
		TimerSwitcher->SetActiveWidgetIndex(TIMER_ACCEPT_INDEX);
		NoButtonSwitcher->SetActiveWidgetIndex(REJECT_BUTTON_INDEX);
		RejectButton->SetIsEnabled(true);
			
		MessageText->SetText(MatchmakingText::Requires_Acceptance);
			
		AcceptTimer = 15;
		bShowTimerTick = true;
		CancelButton->SetIsEnabled(false);
		
		GetWorld()->GetTimerManager().SetTimer(AcceptanceTimeoutHandle, this, &ThisClass::HandleAcceptTimer, 1.0f, true);
		if (IsValid(AcceptButton))
		{
			AcceptButton->SetIsEnabled(true);
		}
			
		if (!PendingTexture || !AcceptedTexture || CurrentState.TotalPlayers == INDEX_NONE) return;

		UpdateAcceptanceProgress(CurrentState.TotalPlayers, CurrentState.AcceptedPlayers);
	}
}

void UPTWServerConnectWidget::HandleFlowPlacing()
{
	UPTWMatchmakingSubsystem* MatchmakingSubsystem = UPTWMatchmakingSubsystem::Get(this);
	if (!IsValid(MatchmakingSubsystem)) return;
	
	const FPTWMatchmakingState& CurrentState = MatchmakingSubsystem->GetCurrentState();
	
	if (TimerSwitcher->GetActiveWidgetIndex() != TIMER_START_INDEX)
	{
		TimerSwitcher->SetActiveWidgetIndex(TIMER_START_INDEX);
	}
	switch (CurrentState.Reason)
	{
	case EPTWMatchmakingReason::PLACING_ACCEPTED:
		MessageText->SetText(MatchmakingText::Placing_Accepted);
		break;
	case EPTWMatchmakingReason::PLACING_REJECTED:
		MessageText->SetText(MatchmakingText::Placing_Rejected);
		break;
	case EPTWMatchmakingReason::PLACING_TIMEOUT:
		MessageText->SetText(MatchmakingText::Placing_Timeout);
		break;
	default:
		MessageText->SetText(MatchmakingText::Placing);
		break;
	}

	GetWorld()->GetTimerManager().ClearTimer(AcceptanceTimeoutHandle);
	if (IsValid(AcceptButton))
	{
		AcceptButton->SetIsEnabled(false);
	}
	if (CurrentState.Reason == EPTWMatchmakingReason::PLACING_ACCEPTED)
	{
		UpdateAcceptanceProgress(CurrentState.TotalPlayers, CurrentState.TotalPlayers);
	}
	bShowTimerTick = true;
	CancelButton->SetIsEnabled(true);
}

void UPTWServerConnectWidget::HandleFlowCompleted()
{
	MessageText->SetText(MatchmakingText::Completed);
	bShowTimerTick = false;
	CancelButton->SetIsEnabled(false);
}

void UPTWServerConnectWidget::HandleFlowTimedOut()
{
	MessageText->SetText(MatchmakingText::Timed_Out);
	bShowTimerTick = false;
	CancelButton->SetIsEnabled(true);
}

void UPTWServerConnectWidget::HandleFlowCancelled()
{
	UPTWMatchmakingSubsystem* MatchmakingSubsystem = UPTWMatchmakingSubsystem::Get(this);
	if (!IsValid(MatchmakingSubsystem)) return;
	
	const FPTWMatchmakingState& CurrentState = MatchmakingSubsystem->GetCurrentState();
	
	switch (CurrentState.Reason)
	{
	case EPTWMatchmakingReason::CANCELLED_REJECTED:
		MessageText->SetText(MatchmakingText::Cancelled_Rejected);
		break;
	case EPTWMatchmakingReason::CANCELLED_ACCEPT_TIMEOUT:
		MessageText->SetText(MatchmakingText::Cancelled_Accept_Timeout);
		break;
	default:
		MessageText->SetText(MatchmakingText::Cancelled);
		break;
	}
	bShowTimerTick = false;
	CancelButton->SetIsEnabled(true);
}

void UPTWServerConnectWidget::HandleFlowFailed()
{
	MessageText->SetText(MatchmakingText::Failed);
	bShowTimerTick = false;
	CancelButton->SetIsEnabled(true);
}

void UPTWServerConnectWidget::OnClickedAccept()
{
	AcceptButton->SetIsEnabled(false);
	
	FString TicketId = FString();
	if (UPTWMatchmakingSubsystem* MatchmakingSubsystem = UPTWMatchmakingSubsystem::Get(this))
	{
		TicketId = MatchmakingSubsystem->GetCurrentState().TicketId;
	}
	
	if (TicketId.IsEmpty()) return;
	if (UPTWGameLiftClientSubsystem* GameLiftSubsystem = UPTWGameLiftClientSubsystem::Get(this))
	{
		GameLiftSubsystem->AcceptMatchmaking(TicketId, true);
	}
}

void UPTWServerConnectWidget::OnClickedCancel()
{
	CancelButton->SetIsEnabled(false);
	
	UPTWMatchmakingSubsystem* MatchmakingSubsystem = UPTWMatchmakingSubsystem::Get(this);
	if (!IsValid(MatchmakingSubsystem)) return;
	
	FString TicketId = MatchmakingSubsystem->GetCurrentState().TicketId;
	EPTWMatchmakingFlow CurrentFlow = MatchmakingSubsystem->GetCurrentState().Flow;
	EPTWMatchmakingReason CurrentReason = MatchmakingSubsystem->GetCurrentState().Reason;
	
	UPTWGameLiftClientSubsystem* GameLiftSubsystem = UPTWGameLiftClientSubsystem::Get(this);
	if (!IsValid(GameLiftSubsystem)) return;

	if (TicketId.IsEmpty())
	{
		MatchmakingSubsystem->HideWidget();
	}
	else
	{
		GameLiftSubsystem->StopMatchmaking(TicketId);
	}
}

void UPTWServerConnectWidget::OnClickedReject()
{
	RejectButton->SetIsEnabled(false);
	
	UPTWMatchmakingSubsystem* MatchmakingSubsystem = UPTWMatchmakingSubsystem::Get(this);
	if (!IsValid(MatchmakingSubsystem)) return;
	
	FString TicketId = MatchmakingSubsystem->GetCurrentState().TicketId;
	if (TicketId.IsEmpty()) return;

	if (UPTWGameLiftClientSubsystem* GameLiftSubsystem = UPTWGameLiftClientSubsystem::Get(this))
	{
		GameLiftSubsystem->AcceptMatchmaking(TicketId, false);
	}
}
