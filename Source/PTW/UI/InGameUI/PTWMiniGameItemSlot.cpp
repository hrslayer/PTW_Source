// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWMiniGameItemSlot.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

#include "Inventory/Instance/PTWItemInstance.h"
#include "Inventory/PTWItemDefinition.h"
#include "Inventory/Instance/PTWActiveItemInstance.h"
#include "Inventory/Instance/PTWWeaponInstance.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"

void UPTWMiniGameItemSlot::SetItemInstance(UPTWItemInstance* InItem)
{
	ItemInstance = InItem;

	if (!ItemInstance)
	{
		ClearSlot();
		return;
	}

	UPTWItemDefinition* ItemDef = ItemInstance->GetItemDef();
	if (!ItemDef) return;

	// 무기 인스턴스인지 확인
	bool bIsWeapon = InItem->IsA(UPTWWeaponInstance::StaticClass());
	ESlateVisibility WeaponUIVisibility = bIsWeapon ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;

	if (SlotNumberText)
	{
		SlotNumberText->SetVisibility(WeaponUIVisibility);
	}

	if (WeaponNameText)
	{
		WeaponNameText->SetVisibility(WeaponUIVisibility);
		if (bIsWeapon)
		{
			// 무기일 경우 이름 설정
			WeaponNameText->SetText(ItemDef->DisplayName);
		}
	}

	// 아이콘
	if (ItemIcon)
	{
		UTexture2D* Icon = ItemDef->ItemIcon.LoadSynchronous();
		if (Icon)
		{
			ItemIcon->SetBrushFromTexture(Icon);
			ItemIcon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			// 이미지가 없으면 Collapsed 처리
			ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// UI 초기화
	if (CooldownProgressBar)
	{
		CooldownProgressBar->SetVisibility(ESlateVisibility::Hidden);
	}

	if (CooldownText)
	{
		CooldownText->SetVisibility(ESlateVisibility::Hidden);
	}

	if (CountText)
	{
		CountText->SetVisibility(ESlateVisibility::Hidden);
	}

	// 쿨타임 아이템
	if (ItemDef->CooldownTag.IsValid())
	{
		CooldownProgressBar->SetVisibility(ESlateVisibility::Visible);
		CooldownText->SetVisibility(ESlateVisibility::Visible);
	}

	// 액티브 아이템인 경우 델리게이트 바인딩
	if (UPTWActiveItemInstance* ActiveItem = Cast<UPTWActiveItemInstance>(InItem))
	{
		// 델리게이트 구독
		ActiveItem->OnCurrentCountChanged.RemoveAll(this);
		ActiveItem->OnCurrentCountChanged.AddUObject(this, &UPTWMiniGameItemSlot::UpdateCount);

		// 초기값 반영
		UpdateCount(ActiveItem->GetCurrentCount());
	}
}

void UPTWMiniGameItemSlot::InitCooldown(
	UAbilitySystemComponent* ASC,
	const FGameplayTag& InCooldownTag)
{
	if (!ASC || !InCooldownTag.IsValid()) return;

	AbilitySystemComponent = ASC;
	CooldownTag = InCooldownTag;

	CooldownTagDelegateHandle =
		AbilitySystemComponent
		->RegisterGameplayTagEvent(
			CooldownTag,
			EGameplayTagEventType::NewOrRemoved)
		.AddUObject(
			this,
			&UPTWMiniGameItemSlot::OnCooldownTagChanged);

	if (AbilitySystemComponent->GetTagCount(CooldownTag) > 0)
	{
		OnCooldownTagChanged(CooldownTag, 1);
	}
}

void UPTWMiniGameItemSlot::UpdateCount(int32 NewCount)
{
	if (!CountText) return;

	int32 DisplayCount = NewCount + 1;

	if (DisplayCount <= 0)
	{
		CountText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	CountText->SetVisibility(ESlateVisibility::Visible);
	CountText->SetText(FText::AsNumber(DisplayCount));
}

void UPTWMiniGameItemSlot::ClearSlot()
{
	if (UPTWActiveItemInstance* ActiveItem = Cast<UPTWActiveItemInstance>(ItemInstance))
	{
		ActiveItem->OnCurrentCountChanged.RemoveAll(this);
	}

	ItemInstance = nullptr;

	// 무기 전용 UI 숨기기
	if (SlotNumberText) SlotNumberText->SetVisibility(ESlateVisibility::Collapsed);
	if (WeaponNameText) WeaponNameText->SetVisibility(ESlateVisibility::Collapsed);

	if (ItemIcon)
	{
		ItemIcon->SetBrushFromTexture(nullptr);
		ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (CooldownProgressBar)
	{
		CooldownProgressBar->SetVisibility(ESlateVisibility::Hidden);
		CooldownProgressBar->SetPercent(0.f);
	}

	if (CooldownText)
	{
		CooldownText->SetVisibility(ESlateVisibility::Hidden);
	}

	if (CountText)
	{
		CountText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPTWMiniGameItemSlot::ResetCooldownUI()
{
	if (CooldownProgressBar)
	{
		CooldownProgressBar->SetPercent(0.f);
	}

	if (CooldownText)
	{
		CooldownText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPTWMiniGameItemSlot::SetSlotNumber(int32 Number)
{
	if (SlotNumberText)
	{
		SlotNumberText->SetText(FText::AsNumber(Number));
	}
}

void UPTWMiniGameItemSlot::SetWeaponDisplayMode(bool bShowFull)
{
	ESlateVisibility TargetVisibility = bShowFull ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	// if (ItemIcon) ItemIcon->SetVisibility(TargetVisibility);
	if (WeaponNameText) WeaponNameText->SetVisibility(TargetVisibility);
}

void UPTWMiniGameItemSlot::SetHighlight(bool bIsSelected)
{
	if (SlotNumberText)
	{
		// 선택된 경우 노란색, 아니면 기본 흰색
		FLinearColor TargetColor = bIsSelected ? FLinearColor(1.f, 0.85f, 0.f) : FLinearColor::White;

		SlotNumberText->SetColorAndOpacity(FSlateColor(TargetColor));
	}
}

void UPTWMiniGameItemSlot::OnCooldownTagChanged(
	const FGameplayTag Tag,
	int32 NewCount)
{
	if (NewCount > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(
			CooldownUpdateTimer,
			this,
			&UPTWMiniGameItemSlot::UpdateCooldownDisplay,
			0.05f,
			true);

		CooldownText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(CooldownUpdateTimer);

		CooldownProgressBar->SetPercent(0.f);

		CooldownText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPTWMiniGameItemSlot::UpdateCooldownDisplay()
{
	if (!AbilitySystemComponent) return;

	FGameplayEffectQuery Query =
		FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(
			FGameplayTagContainer(CooldownTag));

	TArray<TPair<float, float>> DurationAmounts =
		AbilitySystemComponent
		->GetActiveEffectsTimeRemainingAndDuration(Query);

	if (DurationAmounts.Num() > 0)
	{
		float RemainingTime = DurationAmounts[0].Key;
		float TotalDuration = DurationAmounts[0].Value;

		if (TotalDuration > 0.f)
		{
			float Percent =
				FMath::Clamp(RemainingTime / TotalDuration, 0.f, 1.f);

			CooldownProgressBar->SetPercent(Percent);
		}

		FNumberFormattingOptions Format;
		Format.SetMinimumFractionalDigits(1);
		Format.SetMaximumFractionalDigits(1);

		CooldownText->SetText(
			FText::AsNumber(RemainingTime, &Format));
	}
}

void UPTWMiniGameItemSlot::NativeDestruct()
{
	// 아이템 델리게이트 해제
	if (UPTWActiveItemInstance* ActiveItem = Cast<UPTWActiveItemInstance>(ItemInstance))
	{
		ActiveItem->OnCurrentCountChanged.RemoveAll(this);
	}

	// GAS 해제
	if (AbilitySystemComponent && CooldownTagDelegateHandle.IsValid())
	{
		AbilitySystemComponent
			->RegisterGameplayTagEvent(
				CooldownTag,
				EGameplayTagEventType::NewOrRemoved)
			.Remove(CooldownTagDelegateHandle);
	}

	GetWorld()->GetTimerManager().ClearTimer(CooldownUpdateTimer);

	Super::NativeDestruct();
}
