// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "PTWRoundEventTitle.generated.h"

class UTextBlock;
class UDataTable;

/**
 * 
 */
UCLASS()
class PTW_API UPTWRoundEventTitle : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateTitleByPhase(EPTWGamePhase CurrentGamePhase);
	void UpdateTitleByRoulette(const FPTWRouletteData& RouletteData);

protected:
	/** 이벤트 이름 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> EventTitleText;

	/** 이벤트 상세 설명 텍스트 (필요 시) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EventDescriptionText;

	/** 라운드 이벤트 데이터 테이블 */
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UDataTable> RoundEventTable;

private:
	/** RowName을 통해 실제 출력용 이름을 가져오는 헬퍼 함수 */
	FText GetEventDisplayName(FName RowName);

	/** RowName을 통해 상세 설명을 가져오는 헬퍼 함수 */
	FText GetEventDescription(FName RowName);
};
