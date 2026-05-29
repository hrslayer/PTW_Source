// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWRoundEventRoulette.generated.h"

class UTextBlock;
class UDataTable;
/**
 * 
 */
UCLASS()
class PTW_API UPTWRoundEventRoulette : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 데이터 테이블에서 이벤트 정보(이름, 설명)를 캐싱 */
	void CacheEventData();

	/** 텍스트를 무작위로 변경하는 셔플 애니메이션 */
	void PlayShuffleAnim();

	/** 서버에서 확정된 최종 이벤트를 표시 */
	void ShowFinalResult();

protected:
	/** 라운드 이벤트 데이터 테이블 (에디터에서 할당) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	UDataTable* RoundEventTable;

	/** 이벤트 이름이 표시될 텍스트 블록 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* EventNameText;

	/** 이벤트 설명이 표시될 텍스트 블록 (선택 사항) */
	UPROPERTY(meta = (BindWidget), EditAnywhere, BlueprintReadOnly, Category = "UI")
	UTextBlock* DescriptionText;

	/** 셔플 간격 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	float ShuffleInterval = 0.1f;

private:
	/** 셔플 시 보여줄 이름 목록 */
	TArray<FText> CachedEventNames;

	FTimerHandle ShuffleTimerHandle;
};
