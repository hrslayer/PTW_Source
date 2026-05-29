// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWAmmoWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "AbilitySystemComponent.h" // ASC
#include "GAS/PTWWeaponAttributeSet.h" 
#include "GameplayTagContainer.h"


void UPTWAmmoWidget::InitWithASC(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;

	if (AbilitySystemComponent)
	{
		UnBindGASDelegates();
	}

	AbilitySystemComponent = ASC;

	BindGASDelegates(ASC);

	/* 위젯 초기 표시값 설정 */
	float CurrentAmmo = ASC->GetNumericAttribute(UPTWWeaponAttributeSet::GetCurrentAmmoAttribute());
	float MaxAmmo = ASC->GetNumericAttribute(UPTWWeaponAttributeSet::GetMaxAmmoAttribute());

	OnOverheatTagChanged(OverheatTag, ASC->GetTagCount(OverheatTag));

	UpdateAmmoWidget(CurrentAmmo, MaxAmmo);

	const int32 TagCount = ASC->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip")));
	SetAmmoWidgetVisibility(FGameplayTag(), TagCount);
}

void UPTWAmmoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CurrentAmmoText) OriginalTextColor = CurrentAmmoText->GetColorAndOpacity();

	OverheatTag = FGameplayTag::RequestGameplayTag(FName("Weapon.State.Overheated"));
}

void UPTWAmmoWidget::NativeDestruct()
{
	UnBindGASDelegates();

	Super::NativeDestruct();
}

void UPTWAmmoWidget::BindGASDelegates(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;

	/* CurrentAmmo 변경 감지 */
	CurrentAmmoChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(UPTWWeaponAttributeSet::GetCurrentAmmoAttribute())
		.AddUObject(this, &UPTWAmmoWidget::OnCurrentAmmoAttributeChanged);

	/* MaxAmmo 변경 감지 */
	MaxAmmoChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(UPTWWeaponAttributeSet::GetMaxAmmoAttribute())
		.AddUObject(this, &UPTWAmmoWidget::OnMaxAmmoAttributeChanged);

	/* 무기 장착 여부 감지 */
	EquipWeaponHandle =
		ASC->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip")), EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UPTWAmmoWidget::SetAmmoWidgetVisibility);

	/* 과열 태그 여부 감지 */
	OverheatTagChangedHandle = ASC->RegisterGameplayTagEvent(OverheatTag, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UPTWAmmoWidget::OnOverheatTagChanged);
}

void UPTWAmmoWidget::UnBindGASDelegates()
{
	if (AbilitySystemComponent)
	{
		// Attribute 델리게이트 해제
		if (CurrentAmmoChangedHandle.IsValid())
		{
			AbilitySystemComponent
				->GetGameplayAttributeValueChangeDelegate(UPTWWeaponAttributeSet::GetCurrentAmmoAttribute())
				.Remove(CurrentAmmoChangedHandle);
			CurrentAmmoChangedHandle.Reset();
		}

		if (MaxAmmoChangedHandle.IsValid())
		{
			AbilitySystemComponent
				->GetGameplayAttributeValueChangeDelegate(UPTWWeaponAttributeSet::GetMaxAmmoAttribute())
				.Remove(MaxAmmoChangedHandle);
			MaxAmmoChangedHandle.Reset();
		}

		// Tag Event 델리게이트 해제
		if (EquipWeaponHandle.IsValid())
		{
			AbilitySystemComponent
				->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip")), EGameplayTagEventType::NewOrRemoved)
				.Remove(EquipWeaponHandle);
			EquipWeaponHandle.Reset();
		}

		if (OverheatTagChangedHandle.IsValid())
		{
			AbilitySystemComponent->RegisterGameplayTagEvent(OverheatTag, EGameplayTagEventType::NewOrRemoved)
				.Remove(OverheatTagChangedHandle);
			OverheatTagChangedHandle.Reset();
		}
	}
}

void UPTWAmmoWidget::OnCurrentAmmoAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (!AbilitySystemComponent) return;

	float MaxAmmo = AbilitySystemComponent
		->GetNumericAttribute(UPTWWeaponAttributeSet::GetMaxAmmoAttribute());

	UpdateAmmoWidget(Data.NewValue, MaxAmmo);
}


void UPTWAmmoWidget::OnMaxAmmoAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (!AbilitySystemComponent) return;

	float CurrentAmmo = AbilitySystemComponent
		->GetNumericAttribute(UPTWWeaponAttributeSet::GetCurrentAmmoAttribute());

	UpdateAmmoWidget(CurrentAmmo, Data.NewValue);
}

void UPTWAmmoWidget::UpdateAmmoWidget(float CurrentAmmo, float MaxAmmo)
{
	if (CurrentAmmoText)
		CurrentAmmoText->SetText(FText::AsNumber(FMath::RoundToInt(CurrentAmmo)));

	if (MaxAmmoText)
		MaxAmmoText->SetText(FText::AsNumber(FMath::RoundToInt(MaxAmmo)));

	if (AmmoProgressBar)
	{
		// 0으로 나누기 방지 (MaxAmmo가 0일 경우 게이지를 0으로 설정)
		float Percent = (MaxAmmo > 0.0f) ? (CurrentAmmo / MaxAmmo) : 0.0f;

		AmmoProgressBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
	}
}

void UPTWAmmoWidget::SetAmmoWidgetVisibility(const FGameplayTag Tag, int32 NewCount)
{
	const bool bEquipped = (NewCount > 0);
	SetVisibility(bEquipped ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UPTWAmmoWidget::OnOverheatTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	const bool bIsOverheated = NewCount > 0;

	if (bIsOverheated)
	{
		if (CurrentAmmoText) CurrentAmmoText->SetColorAndOpacity(OverheatedColor);
		if (SlashText)       SlashText->SetColorAndOpacity(OverheatedColor);
		if (MaxAmmoText)     MaxAmmoText->SetColorAndOpacity(OverheatedColor);
	}
	else
	{
		if (CurrentAmmoText) CurrentAmmoText->SetColorAndOpacity(OriginalTextColor);
		if (SlashText)       SlashText->SetColorAndOpacity(OriginalTextColor);
		if (MaxAmmoText)     MaxAmmoText->SetColorAndOpacity(OriginalTextColor);
	}
}
