// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RankBoard/PTWResultMVPEntry.h"
#include "Components/TextBlock.h"

void UPTWResultMVPEntry::SetEntryData(const FPTWMiniGameTopResultData& Data)
{
	if (StatNameText)
	{
		// StatData 내부의 StatName에 접근
		StatNameText->SetText(Data.StatData.StatName);
	}

	if (StatValueText)
	{
		// StatData 내부의 StatValue에 접근
		StatValueText->SetText(FText::AsNumber(FMath::RoundToInt(Data.StatData.StatValue)));
	}

	if (PlayerNameText)
	{
		if (Data.PlayerNames.Num() > 0)
		{
			PlayerNameText->SetText(FText::FromString(Data.PlayerNames[0]));
		}
		else
		{
			PlayerNameText->SetText(FText::FromString(TEXT("None")));
		}
	}
}
