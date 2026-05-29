// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h" // FOnAttributeChangeData, FDelegateHandle 사용을 위해 필요
#include "PTWAmmoWidget.generated.h"

class UTextBlock;
class UAbilitySystemComponent;
class UProgressBar;

/**
 * 
 */
UCLASS()
class PTW_API UPTWAmmoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* ASC를 전달받고 초기화 */
	void InitWithASC(UAbilitySystemComponent* ASC);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ASC
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	// 현재 탄창 내 탄약 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentAmmoText;
	// 슬래시 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SlashText;
	// 보유 중인 전체 여분 탄약 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MaxAmmoText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> AmmoProgressBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AmmoWidget|Style")
	FSlateColor OverheatedColor = FLinearColor::Red;

private:
	/* 델리게이트 연결 */
	void BindGASDelegates(UAbilitySystemComponent* ASC);
	/* 델리게이트 해제 */
	void UnBindGASDelegates();

	/* 델리게이트 : 현재탄약 변경 감지 */
	void OnCurrentAmmoAttributeChanged(const FOnAttributeChangeData& Data);
	/* 델리게이트 : 최대탄약 변경 감지 */
	void OnMaxAmmoAttributeChanged(const FOnAttributeChangeData& Data);	
	/* 탄약 위젯 업데이트 */
	void UpdateAmmoWidget(float CurrentAmmo, float ReserveAmmo);
	/* 탄약 위젯 가시여부 */
	void SetAmmoWidgetVisibility(const FGameplayTag Tag, int32 NewCount);
	/* 델리게이트 : 과열 태그 변경 감지 */
	void OnOverheatTagChanged(const FGameplayTag Tag, int32 NewCount);

	/* Delegate Handles (unbind 필요) */
	FDelegateHandle CurrentAmmoChangedHandle;
	FDelegateHandle MaxAmmoChangedHandle;
	FDelegateHandle EquipWeaponHandle;
	FDelegateHandle OverheatTagChangedHandle;

	FSlateColor OriginalTextColor;

	FGameplayTag OverheatTag;
};
