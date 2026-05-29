// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWRoundEventTitle.h"
#include "Components/TextBlock.h"
#include "Engine/DataTable.h"
#include "Event/RoundEvent/PTWRoundEventTable.h"
#include "Event/RoundEvent/PTWRoundEventDefinition.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"

#define LOCTEXT_NAMESPACE "RoundEventTitle"

void UPTWRoundEventTitle::UpdateTitleByPhase(EPTWGamePhase CurrentGamePhase)
{
	if (CurrentGamePhase == EPTWGamePhase::Lobby)
	{
		SetVisibility(ESlateVisibility::Visible);
		// 초기 텍스트 설정
		EventTitleText->SetText(LOCTEXT("WaitingEvent", "라운드이벤트 대기중..."));

		if (EventDescriptionText)
		{
			EventDescriptionText->SetText(FText::GetEmpty());
		}
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPTWRoundEventTitle::UpdateTitleByRoulette(const FPTWRouletteData& RouletteData)
{
	if (!EventTitleText) return;

	if (RouletteData.CurrentPhase == EPTWRoulettePhase::Finished)
	{
		EventTitleText->SetText(GetEventDisplayName(RouletteData.EventRowName));
		if (EventDescriptionText)
		{
			EventDescriptionText->SetText(GetEventDescription(RouletteData.EventRowName));
		}
	}
	else
	{
		EventTitleText->SetText(LOCTEXT("SelectingEvent", "라운드이벤트 선택중..."));

		if (EventDescriptionText)
		{
			EventDescriptionText->SetText(FText::GetEmpty());
		}
	}
}

FText UPTWRoundEventTitle::GetEventDisplayName(FName RowName)
{
	if (!RoundEventTable || RowName.IsNone()) return LOCTEXT("NoneEvent", "None");

	static const FString ContextString(TEXT("RoundEventTitleContext"));
	FPTWRoundEventRow* Row = RoundEventTable->FindRow<FPTWRoundEventRow>(RowName, ContextString);

	if (Row && Row->RoundEvent)
	{
		return Row->RoundEvent->EventName;
	}

	return LOCTEXT("UnknownEvent", "Unknown Event");
}

FText UPTWRoundEventTitle::GetEventDescription(FName RowName)
{
	if (!RoundEventTable || RowName.IsNone()) return FText::GetEmpty();

	static const FString ContextString(TEXT("RoundEventDescContext"));
	FPTWRoundEventRow* Row = RoundEventTable->FindRow<FPTWRoundEventRow>(RowName, ContextString);

	if (Row && Row->RoundEvent)
	{
		return Row->RoundEvent->EventDescription;
	}

	return FText::GetEmpty();
}

#undef LOCTEXT_NAMESPACE
