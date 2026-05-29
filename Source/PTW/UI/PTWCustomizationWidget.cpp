// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PTWCustomizationWidget.h"
#include "Gameplay/Customize/PTWPreviewActor.h"
#include "Components/Button.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Gameplay/Customize/PTWCustomizationSave.h"
#include "UI/PTWUISubsystem.h"

void UPTWCustomizationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), APTWPreviewActor::StaticClass());
	if (FoundActor)
	{
		PreviewCharacter = Cast<APTWPreviewActor>(FoundActor);
	}
	AActor* FoundPawn = UGameplayStatics::GetActorOfClass(GetWorld(), PreviewPawnClass);
	if (FoundPawn)
	{
		PreviewPawn = Cast<APawn>(FoundPawn);
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		OriginalMenuPawn = PC->GetPawn();

		if (PreviewPawn)
		{
			PC->Possess(PreviewPawn);
		}
	}

	for (uint8 i = 1; i <= (uint8)EMeshType::Shoes; ++i)
	{
		CurrentIndices.FindOrAdd((EMeshType)i) = 0;
	}

	if (Btn_Male) Btn_Male->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnMaleClicked);
	if (Btn_Female) Btn_Female->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnFemaleClicked);

	if (Btn_Head_Left) Btn_Head_Left->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnHeadLeftClicked);
	if (Btn_Head_Right) Btn_Head_Right->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnHeadRightClicked);

	if (Btn_Hair_Left) Btn_Hair_Left->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnHairLeftClicked);
	if (Btn_Hair_Right) Btn_Hair_Right->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnHairRightClicked);

	if (Btn_Hat_Left) Btn_Hat_Left->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnHatLeftClicked);
	if (Btn_Hat_Right) Btn_Hat_Right->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnHatRightClicked);

	if (Btn_Eyewear_Left) Btn_Eyewear_Left->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnEyewearLeftClicked);
	if (Btn_Eyewear_Right) Btn_Eyewear_Right->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnEyewearRightClicked);

	if (Btn_Body_Left) Btn_Body_Left->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnBodyLeftClicked);
	if (Btn_Body_Right) Btn_Body_Right->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnBodyRightClicked);

	if (Btn_Gloves_Left) Btn_Gloves_Left->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnGlovesLeftClicked);
	if (Btn_Gloves_Right) Btn_Gloves_Right->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnGlovesRightClicked);

	if (Btn_Lower_Left) Btn_Lower_Left->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnLowerLeftClicked);
	if (Btn_Lower_Right) Btn_Lower_Right->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnLowerRightClicked);

	if (Btn_Shoes_Left) Btn_Shoes_Left->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnShoesLeftClicked);
	if (Btn_Shoes_Right) Btn_Shoes_Right->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnShoesRightClicked);

	if (Btn_UpperAddon_Left) Btn_UpperAddon_Left->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnUpperAddonLeftClicked);
	if (Btn_UpperAddon_Right) Btn_UpperAddon_Right->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnUpperAddonRightClicked);

	if (Btn_BackAddon_Left) Btn_BackAddon_Left->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnBackAddonLeftClicked);
	if (Btn_BackAddon_Right) Btn_BackAddon_Right->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnBackAddonRightClicked);

	if (Btn_BackToMainMenu)
	{
		Btn_BackToMainMenu->OnClicked.AddDynamic(this, &UPTWCustomizationWidget::OnBackToMainMenuClicked);
	}

	FString SaveSlotName = TEXT("CustomSave");
	UPTWCustomizationSave* SaveData = Cast<UPTWCustomizationSave>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));

	if (SaveData)
	{
		CurrentGender = SaveData->EquippedGender;
		RefreshFilteredItems();

		auto FindSavedItemIndex = [this](EMeshType PartType, const FName& SavedID) -> int32
			{
				if (FilteredItems.Contains(PartType))
				{
					int32 FoundIdx = FilteredItems[PartType].Find(SavedID);
					return (FoundIdx != INDEX_NONE) ? FoundIdx : 0;
				}
				return 0;
			};

		CurrentIndices.FindOrAdd(EMeshType::Head) = FindSavedItemIndex(EMeshType::Head, SaveData->EquippedHeadID);
		CurrentIndices.FindOrAdd(EMeshType::Hair) = FindSavedItemIndex(EMeshType::Hair, SaveData->EquippedHairID);
		CurrentIndices.FindOrAdd(EMeshType::Hat) = FindSavedItemIndex(EMeshType::Hat, SaveData->EquippedHatID);
		CurrentIndices.FindOrAdd(EMeshType::Eyewear) = FindSavedItemIndex(EMeshType::Eyewear, SaveData->EquippedEyewearID);
		CurrentIndices.FindOrAdd(EMeshType::Body) = FindSavedItemIndex(EMeshType::Body, SaveData->EquippedBodyID);
		CurrentIndices.FindOrAdd(EMeshType::Gloves) = FindSavedItemIndex(EMeshType::Gloves, SaveData->EquippedGlovesID);
		CurrentIndices.FindOrAdd(EMeshType::Lower) = FindSavedItemIndex(EMeshType::Lower, SaveData->EquippedLowerID);
		CurrentIndices.FindOrAdd(EMeshType::Shoes) = FindSavedItemIndex(EMeshType::Shoes, SaveData->EquippedShoesID);
		CurrentIndices.FindOrAdd(EMeshType::UpperAddon) = FindSavedItemIndex(EMeshType::UpperAddon, SaveData->EquippedUpperAddonID);
		CurrentIndices.FindOrAdd(EMeshType::BackAddon) = FindSavedItemIndex(EMeshType::BackAddon, SaveData->EquippedBackAddonID);
	}
	else
	{
		CurrentGender = EMeshGender::Male;
		RefreshFilteredItems();

		for (uint8 i = 1; i <= (uint8)EMeshType::Shoes; ++i)
		{
			CurrentIndices.FindOrAdd((EMeshType)i) = 0;
		}
	}
	for (uint8 i = 1; i <= (uint8)EMeshType::Shoes; ++i)
	{
		UpdatePreviewMesh((EMeshType)i);
	}
}

void UPTWCustomizationWidget::OnMaleClicked() { SetGender(EMeshGender::Male); }
void UPTWCustomizationWidget::OnFemaleClicked() { SetGender(EMeshGender::Female); }

void UPTWCustomizationWidget::OnHeadLeftClicked() { ShiftPartIndex(EMeshType::Head, false); }
void UPTWCustomizationWidget::OnHeadRightClicked() { ShiftPartIndex(EMeshType::Head, true); }

void UPTWCustomizationWidget::OnHairLeftClicked() { ShiftPartIndex(EMeshType::Hair, false); }
void UPTWCustomizationWidget::OnHairRightClicked() { ShiftPartIndex(EMeshType::Hair, true); }

void UPTWCustomizationWidget::OnHatLeftClicked() { ShiftPartIndex(EMeshType::Hat, false); }
void UPTWCustomizationWidget::OnHatRightClicked() { ShiftPartIndex(EMeshType::Hat, true); }

void UPTWCustomizationWidget::OnEyewearLeftClicked() { ShiftPartIndex(EMeshType::Eyewear, false); }
void UPTWCustomizationWidget::OnEyewearRightClicked() { ShiftPartIndex(EMeshType::Eyewear, true); }

void UPTWCustomizationWidget::OnBodyLeftClicked() { ShiftPartIndex(EMeshType::Body, false); }
void UPTWCustomizationWidget::OnBodyRightClicked() { ShiftPartIndex(EMeshType::Body, true); }

void UPTWCustomizationWidget::OnGlovesLeftClicked() { ShiftPartIndex(EMeshType::Gloves, false); }
void UPTWCustomizationWidget::OnGlovesRightClicked() { ShiftPartIndex(EMeshType::Gloves, true); }

void UPTWCustomizationWidget::OnLowerLeftClicked() { ShiftPartIndex(EMeshType::Lower, false); }
void UPTWCustomizationWidget::OnLowerRightClicked() { ShiftPartIndex(EMeshType::Lower, true); }

void UPTWCustomizationWidget::OnShoesLeftClicked() { ShiftPartIndex(EMeshType::Shoes, false); }
void UPTWCustomizationWidget::OnShoesRightClicked() { ShiftPartIndex(EMeshType::Shoes, true); }

void UPTWCustomizationWidget::OnUpperAddonLeftClicked() { ShiftPartIndex(EMeshType::UpperAddon, false); }
void UPTWCustomizationWidget::OnUpperAddonRightClicked() { ShiftPartIndex(EMeshType::UpperAddon, true); }

void UPTWCustomizationWidget::OnBackAddonLeftClicked() { ShiftPartIndex(EMeshType::BackAddon, false); }
void UPTWCustomizationWidget::OnBackAddonRightClicked() { ShiftPartIndex(EMeshType::BackAddon, true); }


void UPTWCustomizationWidget::SetGender(EMeshGender NewGender)
{
	if (CurrentGender == NewGender) return;
	CurrentGender = NewGender;

	for (auto& Pair : CurrentIndices) Pair.Value = 0;

	RefreshFilteredItems();

	for (uint8 i = 1; i <= (uint8)EMeshType::Shoes; ++i)
	{
		UpdatePreviewMesh((EMeshType)i);
	}
}

void UPTWCustomizationWidget::RefreshFilteredItems()
{
	if (!ItemDataTable) return;
	FilteredItems.Empty();

	TArray<FName> RowNames = ItemDataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FCustomizeDataRow* RowData = ItemDataTable->FindRow<FCustomizeDataRow>(RowName, TEXT("Filter"));
		if (RowData && RowData->Gender == CurrentGender)
		{
			FilteredItems.FindOrAdd(RowData->MeshType).Add(RowName);
		}
	}
}

void UPTWCustomizationWidget::ShiftPartIndex(EMeshType PartType, bool bNext)
{
	if (!FilteredItems.Contains(PartType)) return;

	int32& CurrentIdx = CurrentIndices[PartType];
	int32 MaxIdx = FilteredItems[PartType].Num() - 1;
	if (MaxIdx < 0) return;

	if (bNext) {
		CurrentIdx = (CurrentIdx >= MaxIdx) ? 0 : CurrentIdx + 1;
	}
	else {
		CurrentIdx = (CurrentIdx <= 0) ? MaxIdx : CurrentIdx - 1;
	}

	UpdatePreviewMesh(PartType);
}

void UPTWCustomizationWidget::UpdatePreviewMesh(EMeshType PartType)
{
	if (!PreviewCharacter || !FilteredItems.Contains(PartType)) return;

	TArray<FName>& Items = FilteredItems[PartType];
	int32 Idx = CurrentIndices[PartType];
	if (!Items.IsValidIndex(Idx))
	{
		switch (PartType)
		{
		case EMeshType::Head:       if (Txt_Head_Name)       Txt_Head_Name->SetText(FText::GetEmpty()); break;
		case EMeshType::Hair:       if (Txt_Hair_Name)       Txt_Hair_Name->SetText(FText::GetEmpty()); break;
		case EMeshType::Hat:        if (Txt_Hat_Name)        Txt_Hat_Name->SetText(FText::GetEmpty()); break;
		case EMeshType::Eyewear:    if (Txt_Eyewear_Name)    Txt_Eyewear_Name->SetText(FText::GetEmpty()); break;
		case EMeshType::Body:       if (Txt_Body_Name)       Txt_Body_Name->SetText(FText::GetEmpty()); break;
		case EMeshType::Gloves:     if (Txt_Gloves_Name)     Txt_Gloves_Name->SetText(FText::GetEmpty()); break;
		case EMeshType::Lower:      if (Txt_Lower_Name)      Txt_Lower_Name->SetText(FText::GetEmpty()); break;
		case EMeshType::Shoes:      if (Txt_Shoes_Name)      Txt_Shoes_Name->SetText(FText::GetEmpty()); break;
		case EMeshType::UpperAddon: if (Txt_UpperAddon_Name) Txt_UpperAddon_Name->SetText(FText::GetEmpty()); break;
		case EMeshType::BackAddon:  if (Txt_BackAddon_Name)  Txt_BackAddon_Name->SetText(FText::GetEmpty()); break;
		}
		return;
	}

	FCustomizeDataRow* Data = ItemDataTable->FindRow<FCustomizeDataRow>(Items[Idx], TEXT("Update"));
	if (Data)
	{
		if (PreviewCharacter)
		{
			USkeletalMesh* Mesh = Data->DisplayMesh.LoadSynchronous();
			switch (PartType)
			{
			case EMeshType::Head:       PreviewCharacter->SK_Head->SetSkeletalMesh(Mesh); break;
			case EMeshType::Hair:       PreviewCharacter->SK_Hair->SetSkeletalMesh(Mesh); break;
			case EMeshType::Hat:        PreviewCharacter->SK_Hat->SetSkeletalMesh(Mesh); break;
			case EMeshType::Eyewear:    PreviewCharacter->SK_Eyewear->SetSkeletalMesh(Mesh); break;
			case EMeshType::Body:       PreviewCharacter->BaseMesh->SetSkeletalMesh(Mesh); break; // Body는 보통 메인(Base) 메시를 사용합니다.
			case EMeshType::Gloves:     PreviewCharacter->SK_Gloves->SetSkeletalMesh(Mesh); break;
			case EMeshType::Lower:      PreviewCharacter->SK_Lower->SetSkeletalMesh(Mesh); break;
			case EMeshType::Shoes:      PreviewCharacter->SK_Shoes->SetSkeletalMesh(Mesh); break;
			case EMeshType::UpperAddon: PreviewCharacter->SK_UpperAddon->SetSkeletalMesh(Mesh); break;
			case EMeshType::BackAddon:  PreviewCharacter->SK_BackAddon->SetSkeletalMesh(Mesh); break;
			}
		}
		switch (PartType)
		{
		case EMeshType::Head:       if (Txt_Head_Name)       Txt_Head_Name->SetText(Data->DisplayText); break;
		case EMeshType::Hair:       if (Txt_Hair_Name)       Txt_Hair_Name->SetText(Data->DisplayText); break;
		case EMeshType::Hat:        if (Txt_Hat_Name)        Txt_Hat_Name->SetText(Data->DisplayText); break;
		case EMeshType::Eyewear:    if (Txt_Eyewear_Name)    Txt_Eyewear_Name->SetText(Data->DisplayText); break;
		case EMeshType::Body:       if (Txt_Body_Name)       Txt_Body_Name->SetText(Data->DisplayText); break;
		case EMeshType::Gloves:     if (Txt_Gloves_Name)     Txt_Gloves_Name->SetText(Data->DisplayText); break;
		case EMeshType::Lower:      if (Txt_Lower_Name)      Txt_Lower_Name->SetText(Data->DisplayText); break;
		case EMeshType::Shoes:      if (Txt_Shoes_Name)      Txt_Shoes_Name->SetText(Data->DisplayText); break;
		case EMeshType::UpperAddon: if (Txt_UpperAddon_Name) Txt_UpperAddon_Name->SetText(Data->DisplayText); break;
		case EMeshType::BackAddon:  if (Txt_BackAddon_Name)  Txt_BackAddon_Name->SetText(Data->DisplayText); break;
		}
	}
}

void UPTWCustomizationWidget::OnBackToMainMenuClicked()
{
	SaveCustomizationData();

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (OriginalMenuPawn)
		{
			PC->Possess(OriginalMenuPawn);
		}
	}

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
		{
			if (MainMenuClass)
			{
				UISubsystem->ShowSystemWidget(MainMenuClass);
				UISubsystem->HideSystemWidget(GetClass());
			}
		}
	}
}

void UPTWCustomizationWidget::SaveCustomizationData()
{
	FString SaveSlotName = TEXT("CustomSave");

	UPTWCustomizationSave* SaveData = Cast<UPTWCustomizationSave>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	if (!SaveData)
	{
		SaveData = Cast<UPTWCustomizationSave>(UGameplayStatics::CreateSaveGameObject(UPTWCustomizationSave::StaticClass()));
	}

	SaveData->EquippedGender = CurrentGender;

	auto GetCurrentItemRowName = [this](EMeshType PartType) -> FName
		{
			if (FilteredItems.Contains(PartType) && FilteredItems[PartType].IsValidIndex(CurrentIndices[PartType]))
			{
				return FilteredItems[PartType][CurrentIndices[PartType]];
			}
			return TEXT("None");
		};

	SaveData->EquippedHeadID = GetCurrentItemRowName(EMeshType::Head);
	SaveData->EquippedHairID = GetCurrentItemRowName(EMeshType::Hair);
	SaveData->EquippedHatID = GetCurrentItemRowName(EMeshType::Hat);
	SaveData->EquippedEyewearID = GetCurrentItemRowName(EMeshType::Eyewear);
	SaveData->EquippedBodyID = GetCurrentItemRowName(EMeshType::Body);
	SaveData->EquippedGlovesID = GetCurrentItemRowName(EMeshType::Gloves);
	SaveData->EquippedLowerID = GetCurrentItemRowName(EMeshType::Lower);
	SaveData->EquippedShoesID = GetCurrentItemRowName(EMeshType::Shoes);
	SaveData->EquippedUpperAddonID = GetCurrentItemRowName(EMeshType::UpperAddon);
	SaveData->EquippedBackAddonID = GetCurrentItemRowName(EMeshType::BackAddon);

	if (UGameplayStatics::SaveGameToSlot(SaveData, SaveSlotName, 0))
	{
		UE_LOG(LogTemp, Log, TEXT("성공적으로 커스터마이징 세이브 데이터가 로컬 및 클라우드 준비 슬롯에 저장되었습니다."));
	}
}
