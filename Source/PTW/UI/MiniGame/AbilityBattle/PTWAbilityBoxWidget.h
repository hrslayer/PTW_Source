// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MiniGame/Data/AbilityBattle/PTWTierStyleAsset.h"
#include "PTWAbilityBoxWidget.generated.h"

class URichTextBlock;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDraftSelected, FName);

class UButton;
class UTextBlock;
class UImage;

UCLASS()
class PTW_API UPTWAbilityBoxWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void InitAbilityBoxWidget(FName RowId, UDataTable* DataTable);

	void SetIconTier(EPTWAbilityTier Tier);
	EPTWAbilityTier SelectedTier();

	FOnDraftSelected OnDraftSelected;
private:
	UFUNCTION()
	void OnButtonClicked();

	UFUNCTION()
	void OnButtonHovered();

	UFUNCTION()
	void OnButtonUnHovered();
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_AbilityButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Border;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_AbilityIcon;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_AbilityName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> RichText_AbilityDescription;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UPTWTierStyleAsset> TierStyleAsset;
	
	FName CachedRowId;

	
};
