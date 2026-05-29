// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoreFramework/Game/GameState/PTWGameData.h"
#include "PTWResultMVPEntry.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class PTW_API UPTWResultMVPEntry : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 상위 항목 데이터를 받아 텍스트를 설정하는 함수
	void SetEntryData(const FPTWMiniGameTopResultData& Data);

protected:
	// 스탯 이름 (ex: 가장 많이 생존한 시간)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatNameText;

	// 최고 수치 (ex: 120)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatValueText;

	// 해당 기록을 세운 플레이어 닉네임
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerNameText;
};
