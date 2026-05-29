// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RankBoard/PTWResultStatEntry.h"
#include "Components/TextBlock.h"

void UPTWResultStatEntry::SetStatData(const FText& InName, float InValue)
{
	if (StatNameText)
	{
		StatNameText->SetText(InName);
	}

	if (StatValueText)
	{
		StatValueText->SetText(FText::AsNumber(FMath::RoundToInt(InValue)));
	}
}
