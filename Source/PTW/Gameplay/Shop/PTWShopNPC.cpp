// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Shop/PTWShopNPC.h"
#include "PTWDisplayItem.h"
#include "Net/UnrealNetwork.h"
#include "CoreFramework/PTWPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Shop/PTWShopSpot.h"

APTWShopNPC::APTWShopNPC()
{
	PrimaryActorTick.bCanEverTick = false;
	
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	NPCMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("NPCMesh"));
	NPCMesh->SetupAttachment(RootComponent);

	StandMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StandMesh"));
	StandMesh->SetupAttachment(RootComponent);
	StandMesh->SetCollisionProfileName(TEXT("BlockAll"));

	DecoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DecoMesh"));
	DecoMesh->SetupAttachment(RootComponent);
	DecoMesh->SetCollisionProfileName(TEXT("NoCollision"));
	bReplicates = true;
	bIsLocallyOpen = true;
}

void APTWShopNPC::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() != NM_DedicatedServer)
	{
		CheckShopAvailability();
	}
}

void APTWShopNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (TObjectPtr<APTWDisplayItem> Item : DisplayItems)
	{
		if (IsValid(Item))
		{
			Item->Destroy();
		}
	}
	DisplayItems.Empty();

	Super::EndPlay(EndPlayReason);
}

void APTWShopNPC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APTWShopNPC, ShopCategory);
	DOREPLIFETIME(APTWShopNPC, DisplayItems);
	DOREPLIFETIME(APTWShopNPC, bIsGachaMode);
	DOREPLIFETIME(APTWShopNPC, AssignedSpot);
}

void APTWShopNPC::SetAssignedSpot(APTWShopSpot* InSpot)
{
	AssignedSpot = InSpot;

	if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
	{
		CheckShopAvailability();
	}
}

void APTWShopNPC::CheckShopAvailability()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	APTWPlayerState* PS = PC->GetPlayerState<APTWPlayerState>();
	if (!PS || !AssignedSpot) return;

	bool bIsDepleted = false;

	if (PS->LocalDepletedSpots.Contains(AssignedSpot))
	{
		bIsDepleted = true;
	}
	else
	{
		for (APTWShopSpot* Spot : PS->LocalDepletedSpots)
		{
			if (Spot && Spot->GetName() == AssignedSpot->GetName())
			{
				bIsDepleted = true;
				PS->LocalDepletedSpots.Add(AssignedSpot);
				break;
			}
		}
	}

	if (bIsDepleted)
	{
		CloseShop();
	}
}

void APTWShopNPC::InitializeShop(EShopCategory InCategory, const TArray<FName>& InItemIDs, const TArray<FTransform>& DisplayLocs, bool bInGachaMode)
{
	if (!HasAuthority()) return;

	ShopCategory = InCategory;
	bIsGachaMode = bInGachaMode;

	ApplyCategoryVisuals();

	int32 Count = FMath::Min(InItemIDs.Num(), DisplayLocs.Num());
	for (int32 i = 0; i < Count; ++i)
	{
		FTransform SpawnTM = DisplayLocs[i] * GetActorTransform();

		FActorSpawnParameters Params;
		Params.Owner = this;

		APTWDisplayItem* Item = GetWorld()->SpawnActor<APTWDisplayItem>(DisplayItemClass, SpawnTM, Params);
		if (Item)
		{
			Item->SetParentShop(this);
			Item->InitDisplay(InItemIDs[i], bIsGachaMode);
			DisplayItems.Add(Item);
		}
	}
}

void APTWShopNPC::CloseShop()
{
	for (APTWDisplayItem* Item : DisplayItems)
	{
		if (Item)
		{
			if (USceneComponent* RootComp = Item->GetRootComponent())
			{
				RootComp->SetVisibility(false, true);
			}

			if (HasAuthority())
			{

			}
			else
			{
				if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Item->GetRootComponent()))
				{
					PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}
			}
		}
	}
}

void APTWShopNPC::OnRep_ShopCategory()
{
	ApplyCategoryVisuals();
}

void APTWShopNPC::OnRep_GachaMode()
{
	ApplyCategoryVisuals();
}

void APTWShopNPC::OnRep_AssignedSpot()
{
	CheckShopAvailability();
}

void APTWShopNPC::ApplyCategoryVisuals()
{
	if (bIsGachaMode && GachaMaterial)
	{
		if (StandMesh) StandMesh->SetMaterial(0, GachaMaterial);
		return;
	}
	
	if (TObjectPtr<UMaterialInterface>* FoundMat = CategoryMaterialMap.Find(ShopCategory))
	{
		if (*FoundMat)
		{
			if (StandMesh)
			{
				StandMesh->SetMaterial(0, *FoundMat);
			}
		}
	}
}
