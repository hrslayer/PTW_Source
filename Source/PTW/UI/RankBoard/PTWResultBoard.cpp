// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RankBoard/PTWResultBoard.h"
#include "UI/RankBoard/PTWResultEntry.h"
#include "UI/RankBoard/PTWResultMVPEntry.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerState.h"

void UPTWResultBoard::UpdateResultBoard(const TArray<FPTWMiniGameResultData>& ResultData)
{
	
	if (!ResultListBuffer || EntryClasses.Num() == 0) return;

	// 기존 목록 초기화
	ResultListBuffer->ClearChildren();

	// 자신의 위치 확인
	int32 MyIndex = GetLocalPlayerIndex(ResultData);
	UE_LOG(LogTemp, Log, TEXT("[PTWResult] Local Player Index: %d"), MyIndex);

	int32 DisplayCount = 0;

	// 자신의 정보를 가장 위에 생성
	if (ResultData.IsValidIndex(MyIndex))
	{
		CreateAndAddEntry(ResultData[MyIndex], DisplayCount++);
	}

	// 나머지 플레이어 정보 순회하며 생성
	for (int32 i = 0; i < ResultData.Num(); ++i)
	{
		if (i == MyIndex) continue; // 본인은 이미 추가했으므로 건너뜀
		CreateAndAddEntry(ResultData[i], DisplayCount++);
	}
}

void UPTWResultBoard::UpdateTopResults(const TArray<FPTWMiniGameTopResultData>& TopData)
{
	if (!TopResultListBuffer || TopEntryClasses.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PTWResult] TopResultListBuffer is NULL or TopEntryClasses is empty!"));
		return;
	}

	TopResultListBuffer->ClearChildren();

	for (int32 i = 0; i < TopData.Num(); ++i)
	{
		CreateAndAddTopEntry(TopData[i], i);
	}
}

int32 UPTWResultBoard::GetLocalPlayerIndex(const TArray<FPTWMiniGameResultData>& ResultData)
{
	 APlayerController* PC = GetOwningPlayer();
	 if (!PC || !PC->PlayerState) return INDEX_NONE;
	
	 FString MyPlayerName = PC->PlayerState->GetPlayerName();

	 for (int32 i = 0; i < ResultData.Num(); ++i)
	 {
		 // 이름이 일치하는 인덱스를 반환합니다.
		 if (ResultData[i].PlayerName == MyPlayerName)
		 {
			 return i;
		 }
	 }

	return INDEX_NONE;
}

void UPTWResultBoard::CreateAndAddEntry(const FPTWMiniGameResultData& Data, int32 DisplayIndex)
{
	int32 ClassIndex = DisplayIndex % EntryClasses.Num();
	TSubclassOf<UPTWResultEntry> SelectedClass = EntryClasses[ClassIndex];

	if (SelectedClass)
	{
		UPTWResultEntry* NewEntry = CreateWidget<UPTWResultEntry>(this, SelectedClass);
		if (NewEntry)
		{
			NewEntry->SetEntryData(Data);
			ResultListBuffer->AddChild(NewEntry);

			if (UVerticalBoxSlot* VertSlot = Cast<UVerticalBoxSlot>(NewEntry->Slot))
			{
				VertSlot->SetPadding(FMargin(0.f, 5.f));
			}
		}
	}
}

void UPTWResultBoard::CreateAndAddTopEntry(const FPTWMiniGameTopResultData& Data, int32 DisplayIndex)
{
	int32 ClassIndex = DisplayIndex % TopEntryClasses.Num();
	TSubclassOf<UPTWResultMVPEntry> SelectedClass = TopEntryClasses[ClassIndex];

	if (SelectedClass)
	{
		UPTWResultMVPEntry* NewEntry = CreateWidget<UPTWResultMVPEntry>(this, SelectedClass);
		if (NewEntry)
		{
			NewEntry->SetEntryData(Data);
			TopResultListBuffer->AddChild(NewEntry);

			 if (UVerticalBoxSlot* VertSlot = Cast<UVerticalBoxSlot>(NewEntry->Slot))
			 {
			     VertSlot->SetPadding(FMargin(0.f, 5.f));
			 }
		}
	}
}
