// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Lobby/PTWRoundEventRoulette.h"
#include "Components/TextBlock.h"
#include "Engine/DataTable.h"
#include "Event/RoundEvent/PTWRoundEventTable.h"
#include "Event/RoundEvent/PTWRoundEventDefinition.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"

void UPTWRoundEventRoulette::NativeConstruct()
{
	Super::NativeConstruct();

	// 테이블 데이터 캐싱
	CacheEventData();

	if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
	{
		// 셔플 애니메이션 시작
		GetWorld()->GetTimerManager().SetTimer(ShuffleTimerHandle, this, &ThisClass::PlayShuffleAnim, ShuffleInterval, true);

		// GameState에 설정된 RouletteDuration(5.0초)에 맞춰 결과 표시 타이머 설정
		// 연출을 위해 마지막 1초 전에 멈추도록 설정
		float TotalTime = FMath::Max(0.5f, GS->GetRouletteData().RouletteDuration - 1.0f);

		FTimerHandle ResultTimer;
		GetWorld()->GetTimerManager().SetTimer(ResultTimer, this, &ThisClass::ShowFinalResult, TotalTime, false);
	}
}

void UPTWRoundEventRoulette::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(ShuffleTimerHandle);
	Super::NativeDestruct();
}

void UPTWRoundEventRoulette::CacheEventData()
{
	if (!RoundEventTable) return;

	CachedEventNames.Empty();
	static const FString ContextString(TEXT("EventRouletteContext"));

	TArray<FPTWRoundEventRow*> Rows;
	RoundEventTable->GetAllRows<FPTWRoundEventRow>(ContextString, Rows);

	for (const auto* Row : Rows)
	{
		if (Row && Row->RoundEvent)
		{
			CachedEventNames.Add(Row->RoundEvent->EventName);
		}
	}
}

void UPTWRoundEventRoulette::PlayShuffleAnim()
{
	if (CachedEventNames.Num() == 0 || !EventNameText) return;

	// 무작위로 이름 선택
	int32 RandomIndex = FMath::RandRange(0, CachedEventNames.Num() - 1);
	EventNameText->SetText(CachedEventNames[RandomIndex]);

	// 돌아가는 느낌을 위한 랜덤 색상 연출
	EventNameText->SetColorAndOpacity(FSlateColor(FLinearColor::MakeRandomColor()));

	// 셔플 중에는 설명창을 비우거나 "???"로 표시
	if (DescriptionText)
	{
		DescriptionText->SetText(FText::FromString(TEXT(" ")));
	}
}

void UPTWRoundEventRoulette::ShowFinalResult()
{
	// 셔플 중지
	GetWorld()->GetTimerManager().ClearTimer(ShuffleTimerHandle);

	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
	if (!GS || !RoundEventTable || !EventNameText) return;

	// GameState에서 서버가 확정한 결과 행 이름 가져오기
	FName TargetRowName = GS->GetRouletteData().EventRowName;
	static const FString ContextString(TEXT("FinalEventResultContext"));

	// 데이터 테이블에서 결과 행 찾기
	FPTWRoundEventRow* ResultRow = RoundEventTable->FindRow<FPTWRoundEventRow>(TargetRowName, ContextString);

	if (ResultRow && ResultRow->RoundEvent)
	{
		const UPTWRoundEventDefinition* Def = ResultRow->RoundEvent;

		EventNameText->SetText(Def->EventName);
		EventNameText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));

		if (DescriptionText)
		{
			DescriptionText->SetText(Def->EventDescription);
		}
	}
}
