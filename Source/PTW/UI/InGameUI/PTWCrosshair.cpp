// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWCrosshair.h"
#include "AbilitySystemComponent.h"
#include "Misc/OutputDeviceNull.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "Components/SizeBox.h"
#include "Engine/DataTable.h"
#include "UI/InGameUI/PTWCrosshairData.h"
#include "CoreFramework/PTWGameUserSettings.h"

void UPTWCrosshair::InitWithASC(UAbilitySystemComponent* InASC)
{
	if (!InASC) return;

	if (CachedASC)
	{
		UnBindGASDelegates();
	}

	CachedASC = InASC;

	EquipTag = GameplayTags::Weapon::State::Equip;
	SprintTag = GameplayTags::State::Movement::Sprinting;

	// 델리게이트 등록
	BindGASDelegates(InASC);

	// 초기 상태 반영
	UpdateVisibility();
}

void UPTWCrosshair::UpdateCrosshairSpread(float DynamicSpread, float MaxSpread)
{
	
	FOutputDeviceNull Ar;
	FString Cmd = FString::Printf(TEXT("NativeUpdateCrosshair %f %f"), DynamicSpread, MaxSpread);
	
	CallFunctionByNameWithArguments(*Cmd, Ar, nullptr, true);
}

void UPTWCrosshair::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Hidden);

	BindUserSettingsDelegates();
}

void UPTWCrosshair::NativeDestruct()
{
	UnBindGASDelegates();
	UnBindUserSettingsDelegates();

	Super::NativeDestruct();
}

void UPTWCrosshair::OnStateTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	UpdateVisibility();
}

void UPTWCrosshair::UpdateVisibility()
{
	if (!CachedASC) return;

	bool bHasWeapon = CachedASC->HasMatchingGameplayTag(EquipTag);
	bool bIsSprinting = CachedASC->HasMatchingGameplayTag(SprintTag);

	bool bShouldShow = bHasWeapon && !bIsSprinting;

	SetCrosshairVisibility(bShouldShow);
}

void UPTWCrosshair::SetCrosshairVisibility(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UPTWCrosshair::HandleCrosshairChanged(int32 NewIndex)
{
	if (!CrosshairContainer || !CrosshairDataTable) return;

	CrosshairContainer->ClearChildren();
	CurrentActiveCrosshairWidget = nullptr;

	TArray<FCrosshairData*> AllRows;
	CrosshairDataTable->GetAllRows<FCrosshairData>(TEXT(""), AllRows);

	if (!AllRows.IsValidIndex(NewIndex)) return;

	FCrosshairData* RowData = AllRows[NewIndex];
	if (!RowData) return;

	UClass* WidgetClass = RowData->CrosshairWidgetClass.LoadSynchronous();
	if (WidgetClass)
	{
		CurrentActiveCrosshairWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), WidgetClass);
		if (CurrentActiveCrosshairWidget)
		{
			CurrentActiveCrosshairWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
			CrosshairContainer->AddChild(CurrentActiveCrosshairWidget);
		}
	}
}

void UPTWCrosshair::BindGASDelegates(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;

	EquipTagHandle = ASC->RegisterGameplayTagEvent(EquipTag, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UPTWCrosshair::OnStateTagChanged);

	SprintTagHandle = ASC->RegisterGameplayTagEvent(SprintTag, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UPTWCrosshair::OnStateTagChanged);
}

void UPTWCrosshair::UnBindGASDelegates()
{
	if (CachedASC)
	{
		// Equip 태그 이벤트 해제
		if (EquipTagHandle.IsValid())
		{
			CachedASC->RegisterGameplayTagEvent(EquipTag, EGameplayTagEventType::NewOrRemoved)
				.Remove(EquipTagHandle);
			EquipTagHandle.Reset();
		}

		// Sprint 태그 이벤트 해제
		if (SprintTagHandle.IsValid())
		{
			CachedASC->RegisterGameplayTagEvent(SprintTag, EGameplayTagEventType::NewOrRemoved)
				.Remove(SprintTagHandle);
			SprintTagHandle.Reset();
		}
	}
}

void UPTWCrosshair::BindUserSettingsDelegates()
{
	if (UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings()))
	{
		HandleCrosshairChanged(Settings->GetCrosshairIndex());

		if (!CrosshairChangedHandle.IsValid())
		{
			CrosshairChangedHandle = Settings->OnCrosshairChanged.AddUObject(this, &UPTWCrosshair::HandleCrosshairChanged);
		}
	}
}

void UPTWCrosshair::UnBindUserSettingsDelegates()
{
	if (CrosshairChangedHandle.IsValid())
	{
		if (UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings()))
		{
			Settings->OnCrosshairChanged.Remove(CrosshairChangedHandle);
			CrosshairChangedHandle.Reset();
		}
	}
}
