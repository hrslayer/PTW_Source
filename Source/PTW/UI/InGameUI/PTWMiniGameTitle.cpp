// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWMiniGameTitle.h"
#include "Components/TextBlock.h"
#include "Engine/DataTable.h"

#include "MiniGame/PTWMiniGameMapRow.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "CoreFramework/PTWPlayerState.h"

#define LOCTEXT_NAMESPACE "MiniGameTitle"

void UPTWMiniGameTitle::UpdateTitleByPhase(EPTWGamePhase CurrentGamePhase)
{
	if (!TitleText) return;

	switch (CurrentGamePhase)
	{
	case EPTWGamePhase::Staging:
		TitleText->SetText(LOCTEXT("WaitingStart", "시작 대기중..."));
		break;

	case EPTWGamePhase::Lobby:
		break;

	case EPTWGamePhase::Loading:
		TitleText->SetText(LOCTEXT("Loading", "로딩중..."));
		break;

	case EPTWGamePhase::MiniGame:
	{
		// 미니게임 시작 시 저장된 맵 이름을 가져와 출력
		APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
		if (GS)
		{
			TitleText->SetText(GetMapDisplayName(GS->GetRouletteData().MapRowName));
		}

		UpdateRoleDisplay();
	}
	break;

	case EPTWGamePhase::MiniGameResult:
	case EPTWGamePhase::GameResult:
		break;

	default:
		break;
	}
}

void UPTWMiniGameTitle::UpdateTitleByRoulette(const FPTWRouletteData& RouletteData)
{
	if (!TitleText) return;

	if (RouletteData.CurrentPhase == EPTWRoulettePhase::Finished)
	{
		TitleText->SetText(GetMapDisplayName(RouletteData.MapRowName));

		UpdateRoleDisplay();
	}
	else
	{
		TitleText->SetText(LOCTEXT("SelectingMiniGame", "미니게임 선택중..."));
	}
}

FText UPTWMiniGameTitle::GetMapDisplayName(FName RowName)
{
	if (!MiniGameMapTable || RowName.IsNone())
	{
		return LOCTEXT("UnknownMiniGame", "Unknown MiniGame");
	}

	static const FString ContextString(TEXT("MiniGameTitle"));
	FPTWMiniGameMapRow* Row = MiniGameMapTable->FindRow<FPTWMiniGameMapRow>(RowName, ContextString);

	if (Row)
	{
		return Row->DisplayName;
	}

	return FText::FromName(RowName);
}

void UPTWMiniGameTitle::UpdateRoleDisplay()
{
	if (!RoleText) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	APTWPlayerState* PS = PC->GetPlayerState<APTWPlayerState>();
	if (!PS) return;

	FPTWRoleData RoleData = PS->GetRoleData();

	// 역할 이름이 비어있지 않다면 표시
	if (!RoleData.RoleName.IsEmpty())
	{
		RoleText->SetText(RoleData.RoleName);
		RoleText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		RoleText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

#undef LOCTEXT_NAMESPACE
