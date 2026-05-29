// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWLowBatteryNotification.generated.h"

class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class PTW_API UPTWLowBatteryNotification : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, meta=(BindWidget))
	TObjectPtr<UImage> NotificationBG;
	
	UPROPERTY(EditAnywhere, meta=(BindWidget))
	TObjectPtr<UTextBlock> NotificationText;
	
};
