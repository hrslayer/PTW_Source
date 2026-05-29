// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWPortalCount.h"
#include "Components/TextBlock.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "GameFramework/PlayerState.h"

#define LOCTEXT_NAMESPACE "PTWPortalUI"

void UPTWPortalCount::NativeDestruct()
{
	UnBindAllDelegates();

	Super::NativeDestruct();
}

void UPTWPortalCount::InitWithASC(UAbilitySystemComponent* AbilitySystemComponent)
{
	// 새로운 ASC가 들어오면 기존 ASC 바인딩을 먼저 정리
	if (ASC)
	{
		UnBindASCDelegates();
	}

	ASC = AbilitySystemComponent;

	// ASC 관련 델리게이트 바인딩
	BindASCDelegates();

	InitializeGameState();
}

void UPTWPortalCount::InitializeGameState()
{
	APTWGameState* CurrentGameState = GetWorld() ? GetWorld()->GetGameState<APTWGameState>() : nullptr;

	if (IsValid(CurrentGameState))
	{
		// 재초기화 시 중복 방지
		if (PTWGameState)
		{
			PTWGameState->OnPortalCountChanged.RemoveDynamic(this, &UPTWPortalCount::UpdatePortalText);
		}

		PTWGameState = CurrentGameState;
		GetWorld()->GetTimerManager().ClearTimer(PortalCount_InitGameState);

		// GameState 델리게이트 바인딩
		PTWGameState->OnPortalCountChanged.AddDynamic(this, &UPTWPortalCount::UpdatePortalText);

		// 초기 값 반영
		UpdatePortalText(PTWGameState->GetPortalCurrent(), PTWGameState->GetPortalRequired());
	}
	else
	{
		// GameState를 찾을 때까지 타이머 작동
		if (!PortalCount_InitGameState.IsValid())
		{
			GetWorld()->GetTimerManager().SetTimer(
				PortalCount_InitGameState,
				this,
				&UPTWPortalCount::InitializeGameState,
				0.1f,
				true
			);
		}
	}
}

void UPTWPortalCount::UpdatePortalText(int32 Current, int32 Required)
{
	if (PortalCountText)
	{
		if (!ASC) return;

		bool bIsInPortal = ASC && ASC->HasMatchingGameplayTag(GameplayTags::State::InPortal);

		if (bIsInPortal)
		{
			FText FormattedText = FText::Format(
				LOCTEXT("PortalCountFormat", "{0} / {1}"),
				FText::AsNumber(Current),
				FText::AsNumber(Required)
			);

			PortalCountText->SetText(FormattedText);
		}
		else
		{
			PortalCountText->SetText(
				LOCTEXT("PortalMoveMessage", "포탈로 이동하여 준비완료를 하세요")
			);
		}
	}
}

void UPTWPortalCount::OnPortalTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (PTWGameState)
	{
		UpdatePortalText(PTWGameState->GetPortalCurrent(), PTWGameState->GetPortalRequired());
	}
}

void UPTWPortalCount::BindASCDelegates()
{
	if (!ASC) return;

	// 태그 이벤트 등록 및 핸들 저장
	PortalTagHandle = ASC->RegisterGameplayTagEvent(GameplayTags::State::InPortal, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UPTWPortalCount::OnPortalTagChanged);
}

void UPTWPortalCount::UnBindASCDelegates()
{
	if (ASC && PortalTagHandle.IsValid())
	{
		ASC->RegisterGameplayTagEvent(GameplayTags::State::InPortal, EGameplayTagEventType::NewOrRemoved)
			.Remove(PortalTagHandle);

		PortalTagHandle.Reset();
	}
}

void UPTWPortalCount::UnBindAllDelegates()
{
	// ASC 해제
	UnBindASCDelegates();
	ASC = nullptr;

	// GameState 해제
	if (PTWGameState)
	{
		PTWGameState->OnPortalCountChanged.RemoveDynamic(this, &UPTWPortalCount::UpdatePortalText);
		PTWGameState = nullptr;
	}

	// 타이머 해제
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PortalCount_InitGameState);
	}
}

#undef LOCTEXT_NAMESPACE
