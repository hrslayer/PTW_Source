// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWInventoryWidget.h"
#include "UI/InGameUI/PTWItemSlot.h"

#include "Components/UniformGridPanel.h"
#include "Components/TextBlock.h"

#include "CoreFramework/PTWPlayerState.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "CoreFramework/Game/GameMode/PTWLobbyGameMode.h"
#include "Inventory/PTWItemDefinition.h"
#include "Gameplay/Shop/PTWShopItemData.h" 
#include "Engine/DataTable.h"
#include "System/Shop/PTWShopSubsystem.h"

void UPTWInventoryWidget::InitPS()
{
	APTWPlayerState* PS = GetOwningPlayerState<APTWPlayerState>();
	if (!PS) return;

	if (CachedPlayerState.IsValid())
	{
		UnbindDelegates();
	}

	CachedPlayerState = PS;

	BindDelegates();
}

void UPTWInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPTWInventoryWidget::NativeDestruct()
{
	UnbindDelegates();

	Super::NativeDestruct();
}

void UPTWInventoryWidget::BindDelegates()
{
	/* 델리게이트 바인딩 */
	if (CachedPlayerState.IsValid())
	{
		CachedPlayerState->OnPlayerDataUpdated.AddDynamic(this, &UPTWInventoryWidget::OnInventoryUpdated);
		
		/* 최신 데이터로 초기화 */
		OnInventoryUpdated(CachedPlayerState->GetPlayerData());
	}
}

void UPTWInventoryWidget::UnbindDelegates()
{
	/* 델리게이트 바인딩 해제 */
	if (CachedPlayerState.IsValid())
	{
		CachedPlayerState->OnPlayerDataUpdated.RemoveDynamic(this, &UPTWInventoryWidget::OnInventoryUpdated);
		CachedPlayerState = nullptr;
	}
}

void UPTWInventoryWidget::OnInventoryUpdated(const FPTWPlayerData& NewData)
{
	if (!ItemDataTable || !SlotWidgetClass)
		return;

	if (WeaponGrid) WeaponGrid->ClearChildren();
	if (ActiveItemGrid) ActiveItemGrid->ClearChildren();
	if (PassiveItemGrid) PassiveItemGrid->ClearChildren();

	int32 WeaponIndex = 0;
	int32 ActiveIndex = 0;
	int32 PassiveIndex = 0;

	for (const FString& ItemID : NewData.InventoryItemIDs)
	{
		FName RowName(*ItemID);

		FShopItemRow* Row = ItemDataTable->FindRow<FShopItemRow>(
			RowName,
			TEXT("Inventory Lookup")
		);

		if (!Row)
		{
			UE_LOG(LogTemp, Error, TEXT("ItemID [%s] not found in DataTable"), *ItemID);
			continue;
		}

		UPTWItemDefinition* ItemDef = Row->ItemDefinition.LoadSynchronous();

		if (!ItemDef)
		{
			UE_LOG(LogTemp, Error, TEXT("ItemID [%s] has NULL ItemDefinition"), *ItemID);
			continue;
		}

		/* 아이템 타입에 따라 Grid 분류 */

		switch (ItemDef->ItemType)
		{

		case EItemType::Weapon:
			CreateSlot(WeaponGrid, ItemDef, WeaponIndex++);
			break;

		case EItemType::Active:
			CreateSlot(ActiveItemGrid, ItemDef, ActiveIndex++);
			break;

		case EItemType::Passive:
			CreateSlot(PassiveItemGrid, ItemDef, PassiveIndex++);
			break;

		default:
			break;
		}
	}

	/* 그리드 크기에 따른 텍스트 가시성 업데이트 */
	UpdateTextVisibility();
}

void UPTWInventoryWidget::CreateSlot(UUniformGridPanel* TargetGrid, UPTWItemDefinition* ItemDef, int32 Index)
{
	if (!TargetGrid)
		return;

	UPTWItemSlot* NewSlot = CreateWidget<UPTWItemSlot>(
		this,
		SlotWidgetClass
	);

	if (!NewSlot)
		return;

	NewSlot->SetupSlot(ItemDef);

	int32 RowIdx = Index % MaxRowsPerColumn;
	int32 ColIdx = Index / MaxRowsPerColumn;

	TargetGrid->AddChildToUniformGrid(NewSlot, RowIdx, ColIdx);
}

void UPTWInventoryWidget::UpdateTextVisibility()
{
	if (WeaponText && WeaponGrid)
	{
		ESlateVisibility NewVisibility = (WeaponGrid->GetChildrenCount() > 0) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
		WeaponText->SetVisibility(NewVisibility);
	}

	if (ActiveItemText && ActiveItemGrid)
	{
		ESlateVisibility NewVisibility = (ActiveItemGrid->GetChildrenCount() > 0) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
		ActiveItemText->SetVisibility(NewVisibility);
	}

	if (PassiveItemText && PassiveItemGrid)
	{
		ESlateVisibility NewVisibility = (PassiveItemGrid->GetChildrenCount() > 0) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
		PassiveItemText->SetVisibility(NewVisibility);
	}
}
