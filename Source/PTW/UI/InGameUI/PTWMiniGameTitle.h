// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "PTWMiniGameTitle.generated.h"

class UTextBlock;
class UDataTable;

/**
 * 
 */
UCLASS()
class PTW_API UPTWMiniGameTitle : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateTitleByPhase(EPTWGamePhase CurrentGamePhase);
	void UpdateTitleByRoulette(const FPTWRouletteData& RouletteData);
	
protected:
	/* 미니게임 이름이 표시될 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	/* 역할이 표시될 텍스트 블록 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RoleText;

	/* 맵 정보를 조회하기 위한 데이터 테이블 (에디터에서 할당) */
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UDataTable> MiniGameMapTable;

private:
	/* RowName을 통해 실제 출력용 이름을 가져오는 헬퍼 함수 */
	FText GetMapDisplayName(FName RowName);

	/* PlayerState에서 역할을 가져와 업데이트하는 헬퍼 함수 */
	void UpdateRoleDisplay();
};
