// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWTimer.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UPTWTimer::InitTimer()
{
	APTWGameState* CurrentGameState = GetWorld() ? GetWorld()->GetGameState<APTWGameState>() : nullptr;

	if (IsValid(CurrentGameState))
	{
		if (PTWGameState)
		{
			UnBindGameStateDelegates();
		}

		PTWGameState = CurrentGameState;

		// 2. 델리게이트 바인딩 함수 호출
		BindGameStateDelegates();

		// 초기 값 반영
		HandleRemainTimeChanged(PTWGameState->GetRemainTime());
		MiniGameCountdownChanged(PTWGameState->IsMiniGameCountdown());
	}
	else
	{
		if (!TimerHandle_InitGameState.IsValid())
		{
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle_InitGameState,
				this,
				&UPTWTimer::InitTimer,
				0.1f,
				true
			);
		}
	}
}

void UPTWTimer::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPTWTimer::NativeDestruct()
{
	UnBindGameStateDelegates();

	Super::NativeDestruct();
}

void UPTWTimer::HandleRemainTimeChanged(int32 NewRemainTime)
{
	if (!RemainTimeText) return;

	RemainTimeText->SetText(FormatTime(NewRemainTime));
}

void UPTWTimer::MiniGameCountdownChanged(bool iscountdown)
{
	// false 면 visible
	if (!RemainTimeText) return;

	if (iscountdown)
	{
		RemainTimeText->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		RemainTimeText->SetVisibility(ESlateVisibility::Visible);
	}
}

FText UPTWTimer::FormatTime(int32 Seconds) const
{
	const int32 Min = Seconds / 60;
	const int32 Sec = Seconds % 60;

	return FText::FromString(
		FString::Printf(TEXT("%02d:%02d"), Min, Sec)
	);
}

void UPTWTimer::BindGameStateDelegates()
{
	if (!PTWGameState) return;

	PTWGameState->OnRemainTimeChanged.AddDynamic(this, &UPTWTimer::HandleRemainTimeChanged);
	PTWGameState->OnMiniGameCountdownChanged.AddDynamic(this, &UPTWTimer::MiniGameCountdownChanged);

	// GameState를 찾았으므로 초기화 대기 타이머는 중지
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_InitGameState);
	}
}

void UPTWTimer::UnBindGameStateDelegates()
{
	// 델리게이트 해제
	if (PTWGameState)
	{
		PTWGameState->OnRemainTimeChanged.RemoveDynamic(this, &UPTWTimer::HandleRemainTimeChanged);
		PTWGameState->OnMiniGameCountdownChanged.RemoveDynamic(this, &UPTWTimer::MiniGameCountdownChanged);
		PTWGameState = nullptr;
	}

	// 초기화 대기용 타이머가 돌고 있다면 해제
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_InitGameState);
	}
}
