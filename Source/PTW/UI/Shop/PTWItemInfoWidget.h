// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWItemInfoWidget.generated.h"

class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class PTW_API UPTWItemInfoWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetItemID(FName InItemID);

protected:
	void Refresh();

	/* 이름 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_ItemName;
	/* 아이콘 */
	UPROPERTY(meta = (BindWidget))
	UImage* Image_ItemIcon;
	/* 카테고리 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Category;
	/* 아이템 타입 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_ItemType;
	/* 가격 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Price;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_PriceDelta;
	/* 설명 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Description;

	UPROPERTY()
	FName ItemID;
};
