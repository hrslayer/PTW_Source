// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MiniGame/AbilityBattle/PTWAbilityBoxWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "MiniGame/Data/AbilityBattle/PTWAbilityDefinition.h"
#include "MiniGame/Data/AbilityBattle/PTWAbilityRow.h"


void UPTWAbilityBoxWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_AbilityButton)
	{
		Button_AbilityButton->OnClicked.AddDynamic(this, &UPTWAbilityBoxWidget::OnButtonClicked);
		Button_AbilityButton->OnHovered.AddDynamic(this, &UPTWAbilityBoxWidget::OnButtonHovered);
		Button_AbilityButton->OnUnhovered.AddDynamic(this, &UPTWAbilityBoxWidget::OnButtonUnHovered);
	}

	if (RichText_AbilityDescription)
	{
		RichText_AbilityDescription->SetWrapTextAt(250.f);
	}
}

void UPTWAbilityBoxWidget::InitAbilityBoxWidget(FName RowId, UDataTable* DataTable)
{
	CachedRowId = RowId;
	
	FPTWAbilityRow* Row = DataTable->FindRow<FPTWAbilityRow>(RowId, TEXT(""));
	if (!Row) return;

	UPTWAbilityDefinition* Definition = Row->AbilityDefinition.LoadSynchronous();
	if (!Definition) return;
	
	Image_AbilityIcon->SetBrushFromSoftTexture(Definition->Icon);
	Text_AbilityName->SetText(Definition->Name);

	FText Description = FText::Format(Definition->DescriptionFormat, FText::AsNumber(Definition->StatValue));
	RichText_AbilityDescription->SetText(Description);

	SetIconTier(Row->Tier);

	// 여기서 받아서 rowid 받아서 하는게 아니라 확률에 따라 등급 
}

void UPTWAbilityBoxWidget::SetIconTier(EPTWAbilityTier Tier)
{
	UMaterialInstanceDynamic* MID = Image_Border->GetDynamicMaterial();
	if (!MID) return;

	FPTWTierData* TierData = TierStyleAsset->TierData.Find(Tier);
	if (!TierData) return;
	
	MID->SetVectorParameterValue(TEXT("Tier"), TierData->TierBorderColor);

	if (!Image_AbilityIcon) return;
	
	Image_AbilityIcon->SetColorAndOpacity(TierData->TierBorderColor);
}

EPTWAbilityTier UPTWAbilityBoxWidget::SelectedTier()
{
	float RandValue = FMath::FRandRange(0.f, 1.f);
    
	for (auto& Pair : TierStyleAsset->TierData)
	{
		if (RandValue <= Pair.Value.TierRatio)
			return Pair.Key;
	}
    
	return EPTWAbilityTier::Bronze;
	
}

void UPTWAbilityBoxWidget::OnButtonClicked()
{
	OnDraftSelected.Broadcast(CachedRowId);
}

void UPTWAbilityBoxWidget::OnButtonHovered()
{
	if (!Image_Border) return;

	UMaterialInstanceDynamic* MID = Image_Border->GetDynamicMaterial();
	if (!MID) return;

	MID->SetScalarParameterValue(TEXT("GlowIntensity"), 3.0f);
}

void UPTWAbilityBoxWidget::OnButtonUnHovered()
{
	if (!Image_Border) return;

	UMaterialInstanceDynamic* MID = Image_Border->GetDynamicMaterial();
	if (!MID) return;

	MID->SetScalarParameterValue(TEXT("GlowIntensity"), 1.0f);
}
