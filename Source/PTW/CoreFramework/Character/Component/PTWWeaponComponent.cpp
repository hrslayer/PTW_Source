// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/Character/Component/PTWWeaponComponent.h"

#include "PTWUIControllerComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/PTWWeaponActor.h"
#include "Weapon/PTWWeaponData.h"
#include "GameFramework/Character.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/Instance/PTWWeaponInstance.h"

UPTWWeaponComponent::UPTWWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UPTWWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPTWWeaponComponent, CurrentWeaponTag);
	DOREPLIFETIME(UPTWWeaponComponent, CurrentWeapon);
}

void UPTWWeaponComponent::EquipWeaponByTag(FGameplayTag NewWeaponTag)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	OnRep_CurrentWeaponTag(CurrentWeaponTag);

	if (CurrentWeaponTag == NewWeaponTag)
	{
		if (CurrentWeapon)
		{
			if (FWeaponPair* CurrentPair = SpawnedWeapons.Find(CurrentWeaponTag))
			{
				if (CurrentPair->Weapon1P) CurrentPair->Weapon1P->SetActorHiddenInGame(true);
				if (CurrentPair->Weapon3P) CurrentPair->Weapon3P->SetActorHiddenInGame(true);
			}
			CurrentWeapon = nullptr;
		}
		CurrentWeaponTag = FGameplayTag::EmptyTag;
		return;
	}

	// 기존 무기 숨기기
	if (CurrentWeaponTag.IsValid())
	{
		if (FWeaponPair* OldPair = SpawnedWeapons.Find(CurrentWeaponTag))
		{
			if (OldPair->Weapon1P) OldPair->Weapon1P->SetActorHiddenInGame(true);
			if (OldPair->Weapon3P) OldPair->Weapon3P->SetActorHiddenInGame(true);
		}
	}

	// 새 무기 보이기
	if (FWeaponPair* FoundPair = SpawnedWeapons.Find(NewWeaponTag))
	{
		if (FoundPair->Weapon1P && FoundPair->Weapon3P)
		{
			FoundPair->Weapon1P->SetActorHiddenInGame(false);
			FoundPair->Weapon3P->SetActorHiddenInGame(false);

			CurrentWeapon = FoundPair->Weapon1P;
			CurrentWeaponTag = NewWeaponTag;
		}
	}
}

void UPTWWeaponComponent::AttachWeaponToSocket(APTWWeaponActor* NewWeapon1P, APTWWeaponActor* NewWeapon3P, FGameplayTag WeaponTag)
{
	if (!NewWeapon1P || !NewWeapon3P) return;
	APTWPlayerCharacter* PlayerChar = Cast<APTWPlayerCharacter>(GetOwner());
	if (!PlayerChar) return;

	FWeaponPair Weaponpair;
	Weaponpair.Weapon1P = NewWeapon1P;
	Weaponpair.Weapon3P = NewWeapon3P;
	SpawnedWeapons.Add(WeaponTag, Weaponpair);
	
	FName TargetSocketName = TEXT("WeaponSocket"); // 기본값 (Rifle 등)

	if (WeaponTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Weapon.Gun.Pistol"))))
	{
		TargetSocketName = TEXT("PistolSocket");
	}

	NewWeapon1P->AttachToComponent(PlayerChar->GetMesh1P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetSocketName);
	NewWeapon3P->AttachToComponent(PlayerChar->GetMesh3P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetSocketName);

	NewWeapon1P->ApplyVisualPerspective();
	NewWeapon3P->ApplyVisualPerspective();

	NewWeapon1P->SetActorHiddenInGame(true);
	NewWeapon3P->SetActorHiddenInGame(true);

	NewWeapon1P->SetActorEnableCollision(false);
	NewWeapon3P->SetActorEnableCollision(false);
}

void UPTWWeaponComponent::ApplyRecoil()
{
	if (!CurrentWeapon) return;
	const UPTWWeaponData* Data = CurrentWeapon->GetWeaponData();
	if (!Data) return;

	FGameplayTag FireTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Anim.Fire"));
	if (Data->AnimMap.Contains(FireTag))
	{
		UAnimMontage* Montage = *Data->AnimMap.Find(FireTag);
		if (Montage)
		{
			PlayMontage1P(Montage);
		}
	}
}

float UPTWWeaponComponent::PlayMontage1P(UAnimMontage* MontageToPlay, float PlayRate)
{
	APTWPlayerCharacter* PlayerChar = Cast<APTWPlayerCharacter>(GetOwner());
	if (!PlayerChar || !PlayerChar->IsLocallyControlled() || !PlayerChar->GetMesh1P() || !MontageToPlay)
	{
		return 0.0f;
	}

	UAnimInstance* AnimInstance = PlayerChar->GetMesh1P()->GetAnimInstance();
	if (AnimInstance)
	{
		return AnimInstance->Montage_Play(MontageToPlay, PlayRate);
	}
	return 0.0f;
}

void UPTWWeaponComponent::PlayWeaponMontageByTag(FGameplayTag AnimTag, float PlayRate)
{
	if (!CurrentWeapon) return;

	const UPTWWeaponData* Data = CurrentWeapon->GetWeaponData();
	if (!Data) return;

	if (Data->WeaponAnimMap.Contains(AnimTag))
	{
		UAnimMontage* WeaponMontage = *Data->WeaponAnimMap.Find(AnimTag);
		if (WeaponMontage)
		{
			CurrentWeapon->PlayWeaponMontage(WeaponMontage, PlayRate);
		}
	}
}

void UPTWWeaponComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (CurrentWeapon && CurrentWeapon->GetWeaponItemInstance())
	{
		CurrentWeapon->GetWeaponItemInstance()->RecoverSpread(DeltaTime);
		if (!OwnerPlayerController && !WeaponData && !WeaponInstance)
		{
			SetWeaponSpreadData();
			return;
		}
		
		if (OwnerPlayerController->UIControllerComponent)
		{
			OwnerPlayerController->UIControllerComponent->UpdateCrossHairSpread(WeaponInstance->GetCurrentSpread(), WeaponData->MaxSpread);
		}
	}
}


void UPTWWeaponComponent::OnRep_CurrentWeaponTag(const FGameplayTag& OldTag)
{
	APTWPlayerCharacter* PlayerChar = Cast<APTWPlayerCharacter>(GetOwner());
	if (!PlayerChar) return;

	if (UPTWInventoryComponent* InvComp = PlayerChar->GetInventoryComponent())
	{
		if (OldTag != FGameplayTag::EmptyTag)
		{
			InvComp->WeaponVisibleSetting(OldTag, true);
		}
		InvComp->WeaponVisibleSetting(CurrentWeaponTag, false);
	}
}

void UPTWWeaponComponent::OnRep_CurrentWeapon(APTWWeaponActor* OldWeapon)
{

}

void UPTWWeaponComponent::SetWeaponSpreadData()
{
	APTWPlayerCharacter* OwnerPlayer = Cast<APTWPlayerCharacter>(GetOwner());
	if (!OwnerPlayer) return;
		
	OwnerPlayerController = Cast<APTWPlayerController>(OwnerPlayer->GetController());
	if (!OwnerPlayerController) return;
	WeaponInstance = CurrentWeapon->GetWeaponItemInstance();
	WeaponData = CurrentWeapon->GetWeaponData();
}

