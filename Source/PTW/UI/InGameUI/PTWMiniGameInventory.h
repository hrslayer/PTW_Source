// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWMiniGameInventory.generated.h"

class UUniformGridPanel;
class UPTWMiniGameItemSlot;
class UPTWInventoryComponent;
class UPTWItemInstance;
class UAbilitySystemComponent;

/**
 * 
 */
UCLASS()
class PTW_API UPTWMiniGameInventory : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void InitInventory(UPTWInventoryComponent* InInventory);

protected:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindInventoryDelegates();
	void UnBindInventoryDelegates();

	void RefreshInventory();

	void SetupWeapons(const TArray<UPTWItemInstance*>& WeaponItems);

	void SetupActive(UPTWItemInstance* ActiveItem);
	void EraseActive();

	void SetupPassives(const TArray<UPTWItemInstance*>& PassiveItems);

	UPTWMiniGameItemSlot* CreateSlot();

	void SwitchToActiveState();
	void SwitchToIdleState();

private:

	UPROPERTY()
	TObjectPtr<UPTWInventoryComponent> InventoryComp;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPTWMiniGameItemSlot> ItemSlotClass;

	// 생성된 무기 슬롯들을 저장해둘 배열
	UPROPERTY()
	TArray<UPTWMiniGameItemSlot*> WeaponSlots;

	FTimerHandle IdleTimerHandle;

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> WeaponGrid;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> PassiveGrid;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWMiniGameItemSlot> ActiveItemSlot;
};
