// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RankBoard/PTWResultEntry.h"
#include "UI/RankBoard/PTWResultStatEntry.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h" // 슬롯 설정을 위해 포함

void UPTWResultEntry::SetEntryData(const FPTWMiniGameResultData& Data)
{
	UE_LOG(LogTemp, Log, TEXT("[PTWResult] Setting Entry Data for: %s, Stat Count: %d"), *Data.PlayerName, Data.StatData.Num());

	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(Data.PlayerName));
	}

	UE_LOG(LogTemp, Warning, TEXT("[PTWResult] Received PlayerName: %s"), *Data.PlayerName);

	if (!StatContainer || StatWidgetClasses.Num() == 0) return;

	StatContainer->ClearChildren();

	for (int32 i = 0; i < Data.StatData.Num(); ++i)
	{
		// 클래스들을 순환하며 선택
		int32 ClassIndex = i % StatWidgetClasses.Num();
		TSubclassOf<UPTWResultStatEntry> SelectedClass = StatWidgetClasses[ClassIndex];

		if (SelectedClass)
		{
			UPTWResultStatEntry* StatWidget = CreateWidget<UPTWResultStatEntry>(this, SelectedClass);
			if (StatWidget)
			{
				StatWidget->SetStatData(Data.StatData[i].StatName, Data.StatData[i].StatValue);

				// 가로 상자에 추가하고 슬롯 설정을 가져옵니다.
				UHorizontalBoxSlot* NewSlot = StatContainer->AddChildToHorizontalBox(StatWidget);

				if (NewSlot)
				{
					// 가로 배치 시 스탯 간의 간격을 주기 위해 패딩 설정 (필요에 따라 조절)
					NewSlot->SetPadding(FMargin(5.f, 0.f));

					// 수직 가운데 정렬
					NewSlot->SetVerticalAlignment(VAlign_Center);
				}
			}
		}
	}
}
