// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWSpamAdMainWidget.generated.h"

class UCanvasPanel;
/**
 * 
 */
UCLASS()
class PTW_API UPTWSpamAdMainWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	void StartSpawnSpamAd();
	
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UUserWidget>> SpamAdWidgetClasses;


private:
	void SpawnRandomSpamAd();
	
	FTimerHandle SpawnSpamAdTimerHandle;

	UPROPERTY(EditDefaultsOnly)
	float SpawnSpamAdInterval = 2.f;

	UPROPERTY(EditDefaultsOnly)
	float SpamAdLifeSpan = 3.f;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;

	
};

