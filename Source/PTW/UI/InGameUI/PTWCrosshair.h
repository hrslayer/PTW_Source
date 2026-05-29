// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "PTWCrosshair.generated.h"

class UAbilitySystemComponent;
class USizeBox;
class UDataTable;
/**
 * 
 */
UCLASS()
class PTW_API UPTWCrosshair : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 초기화 시 ASC를 전달받음
	void InitWithASC(UAbilitySystemComponent* InASC);
	
	void UpdateCrosshairSpread(float DynamicSpread, float MaxSpread);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 태그 변경 시 호출될 콜백
	void OnStateTagChanged(const FGameplayTag Tag, int32 NewCount);
	void UpdateVisibility();

	UFUNCTION()
	void SetCrosshairVisibility(bool bVisible);

	// 크로스헤어 이미지 변경 시 호출될 콜백
	void HandleCrosshairChanged(int32 NewIndex);

	/*UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<class UImage> CrosshairImage;*/

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USizeBox> CrosshairContainer;

	// 크로스헤어 이미지 데이터 테이블
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair")
	TObjectPtr<UDataTable> CrosshairDataTable;

private:
	void BindGASDelegates(UAbilitySystemComponent* ASC);
	void UnBindGASDelegates();

	void BindUserSettingsDelegates();
	void UnBindUserSettingsDelegates();

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedASC;

	FGameplayTag EquipTag;
	FGameplayTag SprintTag;

	FDelegateHandle EquipTagHandle;
	FDelegateHandle SprintTagHandle;
	FDelegateHandle CrosshairChangedHandle;

	// 현재 생성되어 컨테이너에 들어가 있는 크로스헤어 WBP 인스턴스
	UPROPERTY()
	TObjectPtr<UUserWidget> CurrentActiveCrosshairWidget;
};
