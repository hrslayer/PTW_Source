// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/Customize/CustomizeData.h"
#include "PTWCustomizationWidget.generated.h"

class APTWPreviewActor;
class UButton;
class UDataTable;
class UTextBlock;
class APawn;

UCLASS()
class PTW_API UPTWCustomizationWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Male;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Female;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Head_Left;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Head_Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Hair_Left;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Hair_Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Hat_Left;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Hat_Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Eyewear_Left;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Eyewear_Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Body_Left;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Body_Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Gloves_Left;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Gloves_Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Lower_Left;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Lower_Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Shoes_Left;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Shoes_Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_UpperAddon_Left;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_UpperAddon_Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_BackAddon_Left;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_BackAddon_Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_BackToMainMenu;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_Head_Name;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_Hair_Name;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_Hat_Name;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_Eyewear_Name;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_Body_Name;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_Gloves_Name;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_Lower_Name;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_Shoes_Name;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_UpperAddon_Name;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_BackAddon_Name;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> MainMenuClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<APawn> PreviewPawnClass;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<APawn> PreviewPawn;

private:
	UFUNCTION() void OnMaleClicked();
	UFUNCTION() void OnFemaleClicked();

	UFUNCTION() void OnHeadLeftClicked();
	UFUNCTION() void OnHeadRightClicked();

	UFUNCTION() void OnHairLeftClicked();
	UFUNCTION() void OnHairRightClicked();

	UFUNCTION() void OnHatLeftClicked();
	UFUNCTION() void OnHatRightClicked();

	UFUNCTION() void OnEyewearLeftClicked();
	UFUNCTION() void OnEyewearRightClicked();

	UFUNCTION() void OnBodyLeftClicked();
	UFUNCTION() void OnBodyRightClicked();

	UFUNCTION() void OnGlovesLeftClicked();
	UFUNCTION() void OnGlovesRightClicked();

	UFUNCTION() void OnLowerLeftClicked();
	UFUNCTION() void OnLowerRightClicked();

	UFUNCTION() void OnShoesLeftClicked();
	UFUNCTION() void OnShoesRightClicked();

	UFUNCTION() void OnUpperAddonLeftClicked();
	UFUNCTION() void OnUpperAddonRightClicked();

	UFUNCTION() void OnBackAddonLeftClicked();
	UFUNCTION() void OnBackAddonRightClicked();

	UFUNCTION()
	void OnBackToMainMenuClicked();

	void SaveCustomizationData();

	void SetGender(EMeshGender NewGender);
	void ShiftPartIndex(EMeshType PartType, bool bNext);
	void RefreshFilteredItems();
	void UpdatePreviewMesh(EMeshType PartType);

	UPROPERTY(EditDefaultsOnly, Category = "Customization")
	TObjectPtr<UDataTable> ItemDataTable;

	EMeshGender CurrentGender = EMeshGender::Male;
	TMap<EMeshType, int32> CurrentIndices;
	TMap<EMeshType, TArray<FName>> FilteredItems;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<APTWPreviewActor> PreviewCharacter;

	UPROPERTY()
	TObjectPtr<APawn> OriginalMenuPawn;
};
