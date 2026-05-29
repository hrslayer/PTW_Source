// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWSpamAdWidget.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class PTW_API UPTWSpamAdWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void RemoveSpamAd();
	
protected:
	virtual void NativeConstruct() override;
	
private:
	UFUNCTION()
	void CloseButtonClick();
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Close;
	
};
