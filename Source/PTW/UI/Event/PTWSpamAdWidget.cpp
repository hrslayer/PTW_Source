// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Event/PTWSpamAdWidget.h"

#include "Components/Button.h"

void UPTWSpamAdWidget::RemoveSpamAd()
{
	RemoveFromParent();
}

void UPTWSpamAdWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Button_Close->OnClicked.AddDynamic(this, &UPTWSpamAdWidget::CloseButtonClick);
}

void UPTWSpamAdWidget::CloseButtonClick()
{
	RemoveSpamAd();
}
