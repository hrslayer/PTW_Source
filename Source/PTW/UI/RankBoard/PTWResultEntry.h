// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoreFramework/Game/GameState/PTWGameData.h"
#include "PTWResultEntry.generated.h"

class UTextBlock;
class UHorizontalBox;
class UPTWResultStatEntry;

/**
 * 
 */
UCLASS()
class PTW_API UPTWResultEntry : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 플레이어 데이터를 받아 텍스트를 구성하는 함수
	void SetEntryData(const FPTWMiniGameResultData& Data);

protected:
	// 플레이어 이름 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerNameText;

	// 스탯 위젯들이 배치될 컨테이너
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> StatContainer;

	// 사용할 스탯 위젯 클래스 (에디터에서 할당)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TArray<TSubclassOf<UPTWResultStatEntry>> StatWidgetClasses;
};
