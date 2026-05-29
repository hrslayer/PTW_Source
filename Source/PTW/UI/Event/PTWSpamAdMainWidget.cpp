// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Event/PTWSpamAdMainWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "CoreFramework/PTWPlayerCharacter.h"


void UPTWSpamAdMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	this->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	
	if (CanvasPanel)
	{
		CanvasPanel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	
}
void UPTWSpamAdMainWidget::NativeDestruct()
{
	Super::NativeDestruct();

	GetWorld()->GetTimerManager().ClearTimer(SpawnSpamAdTimerHandle);
}

void UPTWSpamAdMainWidget::StartSpawnSpamAd()
{
	GetWorld()->GetTimerManager().SetTimer(SpawnSpamAdTimerHandle, this, &UPTWSpamAdMainWidget::SpawnRandomSpamAd, SpawnSpamAdInterval, true, false);
}

void UPTWSpamAdMainWidget::SpawnRandomSpamAd()
{
	TSubclassOf<UUserWidget> RandClass = SpamAdWidgetClasses[FMath::RandRange(0, SpamAdWidgetClasses.Num()-1)];
	if (!RandClass) return;

	UUserWidget* SpamAdWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), RandClass);
	if (!SpamAdWidget) return;
	
	SpamAdWidget->SetVisibility(ESlateVisibility::Hidden);
	
	UCanvasPanelSlot* CanvasPanelSlot = CanvasPanel->AddChildToCanvas(SpamAdWidget);
	CanvasPanelSlot->SetAnchors(FAnchors(0.f, 0.f));
	CanvasPanelSlot->SetAlignment(FVector2D(0.f, 0.f));
	CanvasPanelSlot->SetAutoSize(true);

	GetWorld()->GetTimerManager().SetTimerForNextTick([this, CanvasPanelSlot, SpamAdWidget]()
	{
		FVector2D ActualSize = SpamAdWidget->GetDesiredSize();
		FVector2D CanvasSize = CanvasPanel->GetCachedGeometry().GetLocalSize();

		CanvasPanelSlot->SetPosition(FVector2D(
			FMath::FRandRange(0.f, CanvasSize.X - ActualSize.X),
			FMath::FRandRange(0.f, CanvasSize.Y - ActualSize.Y)
		));

		SpamAdWidget->SetVisibility(ESlateVisibility::Visible);
	});

	FTimerHandle LifespanHandle;
	GetWorld()->GetTimerManager().SetTimer(
		LifespanHandle, 
		[SpamAdWidget]() 
		{
			if (IsValid(SpamAdWidget))
			{
				SpamAdWidget->RemoveFromParent();
			}
		}, 
		SpamAdLifeSpan, 
		false
	);
}


	

