// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoreFramework/Game/GameState/PTWGameData.h"
#include "PTWResultBoard.generated.h"

class UVerticalBox;
class UPTWResultEntry;
class UPTWResultMVPEntry;
/**
 * 
 */
UCLASS()
class PTW_API UPTWResultBoard : public UUserWidget
{
	GENERATED_BODY()

public:
	// 결과 데이터를 받아 UI를 갱신하는 메인 함수
	void UpdateResultBoard(const TArray<FPTWMiniGameResultData>& ResultData);
	void UpdateTopResults(const TArray<FPTWMiniGameTopResultData>& TopData);

protected:
	// 엔트리들이 추가될 스크롤 박스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> ResultListBuffer;
	// 최고 기록이 추가될 가로 박스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> TopResultListBuffer;

	// 엔트리 위젯 클래스 (에디터에서 할당)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TArray<TSubclassOf<UPTWResultEntry>> EntryClasses;
	// MVP 엔트리 위젯 클래스 (에디터에서 할당)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TArray<TSubclassOf<UPTWResultMVPEntry>> TopEntryClasses;

private:
	// 자신의 정보가 배열의 몇 번째 인덱스인지 확인하는 함수
	int32 GetLocalPlayerIndex(const TArray<FPTWMiniGameResultData>& ResultData);

	// 엔트리를 생성하고 데이터를 설정하여 리스트에 추가하는 헬퍼 함수
	void CreateAndAddEntry(const FPTWMiniGameResultData& Data, int32 DisplayIndex);
	// MVP 엔트리를 생성하고 데이터를 설정하여 리스트에 추가하는 헬퍼 함수
	void CreateAndAddTopEntry(const FPTWMiniGameTopResultData& Data, int32 DisplayIndex);
};
