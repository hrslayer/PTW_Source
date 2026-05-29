// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWBaseCharacter.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerState.h" 
#include "Components/CapsuleComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "PTWPlayerState.h"
#include "PTWPlayerController.h"
#include "GAS/PTWGameplayAbility.h"
#include "Game/GameState/PTWGameState.h"
#include "CoreFramework/Character/Component/PTWReactorComponent.h"
#include "MiniGame/GameMode/PTWBombMiniGameMode.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "Gameplay/Customize/CustomizeData.h"
#include "Gameplay/Customize/PTWCustomizationSave.h"

APTWBaseCharacter::APTWBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bAlwaysRelevant = true;

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		CharacterMesh->SetCullDistance(0.0f);
		CharacterMesh->BoundsScale = 2.0f;
		CharacterMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}

	ReactorComponent = CreateDefaultSubobject<UPTWReactorComponent>(TEXT("ReactorComponent"));

	GetMesh()->SetOwnerNoSee(true);
	SK_UpperAddon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("UpperAddon"));
	SK_UpperAddon->SetupAttachment(GetMesh());
	SK_UpperAddon->SetLeaderPoseComponent(GetMesh());
	SK_UpperAddon->SetOwnerNoSee(true);
	SK_BackAddon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BackAddon"));
	SK_BackAddon->SetupAttachment(GetMesh(), FName("back_accessories"));
	SK_BackAddon->SetOwnerNoSee(true);
	SK_Eyewear = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Eyewear"));
	SK_Eyewear->SetupAttachment(GetMesh(), FName("head_accessories"));
	SK_Eyewear->SetOwnerNoSee(true);
	SK_Gloves = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gloves"));
	SK_Gloves->SetupAttachment(GetMesh());
	SK_Gloves->SetLeaderPoseComponent(GetMesh());
	SK_Gloves->SetOwnerNoSee(true);
	SK_Hair = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hair"));
	SK_Hair->SetupAttachment(GetMesh(), FName("head_accessories"));
	SK_Hair->SetOwnerNoSee(true);
	SK_Hat = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hat"));
	SK_Hat->SetupAttachment(GetMesh(), FName("head_accessories"));
	SK_Hat->SetOwnerNoSee(true);
	SK_Head = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Head"));
	SK_Head->SetupAttachment(GetMesh());
	SK_Head->SetLeaderPoseComponent(GetMesh());
	SK_Head->SetOwnerNoSee(true);
	SK_Lower = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Lower"));
	SK_Lower->SetupAttachment(GetMesh());
	SK_Lower->SetLeaderPoseComponent(GetMesh());
	SK_Lower->SetOwnerNoSee(true);
	SK_Shoes = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Shoes"));
	SK_Shoes->SetupAttachment(GetMesh());
	SK_Shoes->SetLeaderPoseComponent(GetMesh());
	SK_Shoes->SetOwnerNoSee(true);
}

bool APTWBaseCharacter::IsDead() const
{
	return IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(GameplayTags::State::Status_Dead);
}

UAbilitySystemComponent* APTWBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APTWBaseCharacter::HandleDeath(AActor* Attacker)
{
	if (!HasAuthority() || !AbilitySystemComponent) return;

	FGameplayEventData Payload;
	Payload.EventTag = GameplayTags::State::Status_Dead;
	Payload.Instigator = Attacker;
	Payload.Target = this;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, Payload.EventTag, Payload);

	if (ReactorComponent)
	{
		ReactorComponent->ProcessDeath();
	}

	AbilitySystemComponent->AddLooseGameplayTag(GameplayTags::State::Status_Dead);

	if (OnCharacterDied.IsBound())
	{
		OnCharacterDied.Broadcast(this, Attacker);
	}

	if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
	{
		AActor* DeadActor = nullptr;
		if (GetPlayerState())
		{
			DeadActor = GetPlayerState();
		}
		else
		{
			DeadActor = this;
		}
		
		if (GetWorld() && GetWorld()->GetAuthGameMode<APTWBombMiniGameMode>())
		{
			return;
		}
		
		GS->Multicast_BroadcastKilllog(DeadActor, Attacker);
	}
}

float APTWBaseCharacter::GetDamageMultiplier(const FName& BoneName) const
{
	if (BoneName == "head")
	{
		return 2.0f;
	}
	
	return 1.0f;
}

void APTWBaseCharacter::RemoveEffectWithTag(const FGameplayTag& TagToRemove)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(TagToRemove));
	}
}

void APTWBaseCharacter::ApplyGameplayEffectToSelf(TSubclassOf<class UGameplayEffect> EffectClass, float Level,
	FGameplayEffectContextHandle Context)
{
	if (AbilitySystemComponent && EffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, Level, Context);
		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void APTWBaseCharacter::ApplyGameplayEffectWithDuration(TSubclassOf<class UGameplayEffect> EffectClass, float Level,
	float Duration, FGameplayEffectContextHandle Context)
{
	if (AbilitySystemComponent && EffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, Level, Context);
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Duration")),
				Duration
				);
			
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	
}

void APTWBaseCharacter::LoadLocalCustomizationAndSendToServer()
{
	UPTWCustomizationSave* LoadedData = Cast<UPTWCustomizationSave>(UGameplayStatics::LoadGameFromSlot(TEXT("CustomSave"), 0));

	FCharacterCustomizationInfo MyInfo;

	if (LoadedData)
	{
		MyInfo.EquippedUpperAddonID = LoadedData->EquippedUpperAddonID;
		MyInfo.EquippedBackAddonID = LoadedData->EquippedBackAddonID;
		MyInfo.EquippedBodyID = LoadedData->EquippedBodyID;
		MyInfo.EquippedEyewearID = LoadedData->EquippedEyewearID;
		MyInfo.EquippedGlovesID = LoadedData->EquippedGlovesID;
		MyInfo.EquippedHairID = LoadedData->EquippedHairID;
		MyInfo.EquippedHatID = LoadedData->EquippedHatID;
		MyInfo.EquippedHeadID = LoadedData->EquippedHeadID;
		MyInfo.EquippedLowerID = LoadedData->EquippedLowerID;
		MyInfo.EquippedShoesID = LoadedData->EquippedShoesID;
	}

	Server_SetCustomizationInfo(MyInfo);
}

void APTWBaseCharacter::Server_SetCustomizationInfo_Implementation(FCharacterCustomizationInfo NewInfo)
{
	CustomizationInfo = NewInfo;

	ApplyCustomizationFromInfo(CustomizationInfo);
}

void APTWBaseCharacter::OnRep_CustomizationInfo()
{
	ApplyCustomizationFromInfo(CustomizationInfo);
}

void APTWBaseCharacter::ApplyCustomizationFromInfo(const FCharacterCustomizationInfo& Info)
{
	UDataTable* CustomDT = LoadObject<UDataTable>(nullptr, TEXT("/Game/_PTW/BluePrints/Gameplay/Customize/DT_CustomizeData"));
	if (!CustomDT) return;

	if (!Info.EquippedUpperAddonID.IsNone())
	{
		FCustomizeDataRow* UpperAddon = CustomDT->FindRow<FCustomizeDataRow>(Info.EquippedUpperAddonID, TEXT("CustomLoad"));
		if (UpperAddon && !UpperAddon->DisplayMesh.IsNull())
		{
			if (UpperAddon->MeshType == EMeshType::UpperAddon)
			{
				USkeletalMesh* LoadedMesh = UpperAddon->DisplayMesh.LoadSynchronous();
				SK_UpperAddon->SetSkeletalMesh(LoadedMesh);
			}
		}
	}
	if (!Info.EquippedBackAddonID.IsNone())
	{
		FCustomizeDataRow* BackAddon = CustomDT->FindRow<FCustomizeDataRow>(Info.EquippedBackAddonID, TEXT("CustomLoad"));
		if (BackAddon && !BackAddon->DisplayMesh.IsNull())
		{
			if (BackAddon->MeshType == EMeshType::BackAddon)
			{
				USkeletalMesh* LoadedMesh = BackAddon->DisplayMesh.LoadSynchronous();
				SK_BackAddon->SetSkeletalMesh(LoadedMesh);
			}
		}
	}
	if (!Info.EquippedBodyID.IsNone())
	{
		FCustomizeDataRow* Body = CustomDT->FindRow<FCustomizeDataRow>(Info.EquippedBodyID, TEXT("CustomLoad"));
		if (Body && !Body->DisplayMesh.IsNull())
		{
			if (Body->MeshType == EMeshType::Body)
			{
				USkeletalMesh* LoadedMesh = Body->DisplayMesh.LoadSynchronous();
				GetMesh()->SetSkeletalMesh(LoadedMesh);
			}
		}
	}

	if (!Info.EquippedEyewearID.IsNone())
	{
		FCustomizeDataRow* Eyewear = CustomDT->FindRow<FCustomizeDataRow>(Info.EquippedEyewearID, TEXT("CustomLoad"));
		if (Eyewear && !Eyewear->DisplayMesh.IsNull())
		{
			if (Eyewear->MeshType == EMeshType::Eyewear)
			{
				USkeletalMesh* LoadedMesh = Eyewear->DisplayMesh.LoadSynchronous();
				SK_Eyewear->SetSkeletalMesh(LoadedMesh);
			}
		}
	}

	if (!Info.EquippedGlovesID.IsNone())
	{
		FCustomizeDataRow* Gloves = CustomDT->FindRow<FCustomizeDataRow>(Info.EquippedGlovesID, TEXT("CustomLoad"));
		if (Gloves && !Gloves->DisplayMesh.IsNull())
		{
			if (Gloves->MeshType == EMeshType::Gloves)
			{
				USkeletalMesh* LoadedMesh = Gloves->DisplayMesh.LoadSynchronous();
				SK_Gloves->SetSkeletalMesh(LoadedMesh);
			}
		}
	}

	if (!Info.EquippedHairID.IsNone())
	{
		FCustomizeDataRow* Hair = CustomDT->FindRow<FCustomizeDataRow>(Info.EquippedHairID, TEXT("CustomLoad"));
		if (Hair && !Hair->DisplayMesh.IsNull())
		{
			if (Hair->MeshType == EMeshType::Hair)
			{
				USkeletalMesh* LoadedMesh = Hair->DisplayMesh.LoadSynchronous();
				SK_Hair->SetSkeletalMesh(LoadedMesh);
			}
		}
	}

	if (!Info.EquippedHatID.IsNone())
	{
		FCustomizeDataRow* Hat = CustomDT->FindRow<FCustomizeDataRow>(Info.EquippedHatID, TEXT("CustomLoad"));
		if (Hat && !Hat->DisplayMesh.IsNull())
		{
			if (Hat->MeshType == EMeshType::Hat)
			{
				USkeletalMesh* LoadedMesh = Hat->DisplayMesh.LoadSynchronous();
				SK_Hat->SetSkeletalMesh(LoadedMesh);
			}
		}
	}

	if (!Info.EquippedHeadID.IsNone())
	{
		FCustomizeDataRow* Head = CustomDT->FindRow<FCustomizeDataRow>(Info.EquippedHeadID, TEXT("CustomLoad"));
		if (Head && !Head->DisplayMesh.IsNull())
		{
			if (Head->MeshType == EMeshType::Head)
			{
				USkeletalMesh* LoadedMesh = Head->DisplayMesh.LoadSynchronous();
				SK_Head->SetSkeletalMesh(LoadedMesh);
			}
		}
	}
	if (!Info.EquippedLowerID.IsNone())
	{
		FCustomizeDataRow* Lower = CustomDT->FindRow<FCustomizeDataRow>(Info.EquippedLowerID, TEXT("CustomLoad"));
		if (Lower && !Lower->DisplayMesh.IsNull())
		{
			if (Lower->MeshType == EMeshType::Lower)
			{
				USkeletalMesh* LoadedMesh = Lower->DisplayMesh.LoadSynchronous();
				SK_Lower->SetSkeletalMesh(LoadedMesh);
			}
		}
	}
	if (!Info.EquippedShoesID.IsNone())
	{
		FCustomizeDataRow* Shoes = CustomDT->FindRow<FCustomizeDataRow>(Info.EquippedShoesID, TEXT("CustomLoad"));
		if (Shoes && !Shoes->DisplayMesh.IsNull())
		{
			if (Shoes->MeshType == EMeshType::Shoes)
			{
				USkeletalMesh* LoadedMesh = Shoes->DisplayMesh.LoadSynchronous();
				SK_Shoes->SetSkeletalMesh(LoadedMesh);
			}
		}
	}
}


void APTWBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority() && IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(GameplayTags::State::Status_Dead))
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(GameplayTags::State::Status_Dead);
	}

	if (IsLocallyControlled())
	{
		LoadLocalCustomizationAndSendToServer();
	}
}

void APTWBaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnCharacterDied.Clear();
	
	Super::EndPlay(EndPlayReason);
}

void APTWBaseCharacter::InitAbilityActorInfo()
{

}

void APTWBaseCharacter::GiveDefaultAbilities()
{
	if (!HasAuthority() || !AbilitySystemComponent) return;

	TArray<FGameplayAbilitySpecHandle> AbilitiesToRemove;

	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		bool bIsNeeded = false;

		for (TSubclassOf<UGameplayAbility> NeededAbilityClass : DefaultAbilities)
		{
			if (NeededAbilityClass && Spec.Ability == NeededAbilityClass->GetDefaultObject())
			{
				bIsNeeded = true;
				break;
			}
		}
		if (!bIsNeeded)
		{
			AbilitiesToRemove.Add(Spec.Handle);
		}
	}

	for (const FGameplayAbilitySpecHandle& Handle : AbilitiesToRemove)
	{
		AbilitySystemComponent->ClearAbility(Handle);
	}

	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			if (AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass))
			{
				continue;
			}
			FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, this);

			if (const UPTWGameplayAbility* PTWAbility = Cast<UPTWGameplayAbility>(AbilityClass->GetDefaultObject()))
			{
				if (PTWAbility->StartupInputTag.IsValid())
				{
					Spec.DynamicAbilityTags.AddTag(PTWAbility->StartupInputTag);
				}
			}
			AbilitySystemComponent->GiveAbility(Spec);
		}
	}
	if (APTWPlayerState* PS = GetPlayerState<APTWPlayerState>())
	{
		PS->ApplyAdditionalAbilities();
	}
}

void APTWBaseCharacter::ApplyDefaultEffects()
{
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent) return;

	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	for (TSubclassOf<UGameplayEffect> EffectClass : DefaultEffects)
	{
		if (!EffectClass) continue;

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.0f, ContextHandle);

		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	if (APTWPlayerState* PS = GetPlayerState<APTWPlayerState>())
	{
		PS->ApplyAdditionalEffects();
	}
}

void APTWBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APTWBaseCharacter, CustomizationInfo);
}
