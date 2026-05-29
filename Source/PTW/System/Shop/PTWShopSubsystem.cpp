// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Shop/PTWShopSubsystem.h"
#include "UObject/ConstructorHelpers.h"
#include "Gameplay/Shop/PTWShopNPC.h"
#include "Gameplay/Shop/PTWShopSpot.h"
#include "Kismet/KismetMathLibrary.h"
#include "CoreFramework/PTWPlayerState.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"

void UPTWShopSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UDataTable* LoadedTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/_PTW/Data/DT_ShopItems"));

	if (LoadedTable)
	{
		MasterShopTable = LoadedTable;
	}

	if (MasterShopTable)
	{
		CachedShopItems.Empty();
		static const FString ContextString(TEXT("ShopInit"));
		for (const FName& Name : MasterShopTable->GetRowNames())
		{
			if (FShopItemRow* Row = MasterShopTable->FindRow<FShopItemRow>(Name, ContextString))
				CachedShopItems.Add(Name, *Row);
		}
	}

	FString NPCPath = TEXT("/Game/_PTW/BluePrints/Gameplay/Shop/BP_PTWShopNPC.BP_PTWShopNPC_C");

	UClass* LoadedNPCClass = LoadClass<APTWShopNPC>(nullptr, *NPCPath);

	if (LoadedNPCClass)
	{
		ShopNPCClass = LoadedNPCClass;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load ShopNPC Class at path: %s"), *NPCPath);
	}
}

void UPTWShopSubsystem::RegisterShopSpot(APTWShopSpot* Spot)
{
	if (Spot && !ShopSpots.Contains(Spot)) ShopSpots.Add(Spot);
}

void UPTWShopSubsystem::UnregisterShopSpot(APTWShopSpot* Spot)
{
	ShopSpots.Remove(Spot);
}

void UPTWShopSubsystem::InitializeShopsForRound(FGameplayTag NextMinigameTag, FGameplayTag RoundEventTag)
{
	if (ShopSpots.Num() == 0 || !ShopNPCClass) return;

	if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
	{
		GS->SetCurrentRoundEventTag(RoundEventTag);
	}

	for (const auto& NPC : ActiveNPCs) { if (IsValid(NPC)) NPC->Destroy(); }
	ActiveNPCs.Empty();
	GlobalItemStock.Empty();
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	CurrentCrowdFundingAmount = 0;
	bIsShopOpenedByFunding = false;

	if (RoundEventTag.MatchesTag(GameplayTags::Event::Round::Shop::CrowdFunding))
	{
		GetWorld()->GetTimerManager().SetTimer(CrowdFundingTimerHandle, this, &UPTWShopSubsystem::OnCrowdFundingFailed, 30.0f, false);
		return;
	}

	ReShuffleShops();
}

int32 UPTWShopSubsystem::GetItemPrice(FName ItemID) const
{
	if (GetCurrentRoundEventTag().MatchesTag(GameplayTags::Event::Round::Shop::Gacha))
	{
		return 300;
	}

	if (const FShopItemRow* ItemData = CachedShopItems.Find(ItemID))
	{
		float FinalPrice = (float)ItemData->BasePrice * GetPriceMultiplier();
		return FMath::Max(1, FMath::RoundToInt(FinalPrice));
	}
	return 999999;
}

float UPTWShopSubsystem::GetPriceMultiplier() const
{
	if (GetCurrentRoundEventTag().MatchesTag(GameplayTags::Event::Round::Shop::Inflation))
	{
		return 3.0f;
	}
	if (GetCurrentRoundEventTag().MatchesTag(GameplayTags::Event::Round::Shop::FlashSale))
	{
		return 0.5f;
	}
	return 1.0f;
}

void UPTWShopSubsystem::ReShuffleShops()
{
	for (const auto& NPC : ActiveNPCs) { if (IsValid(NPC)) NPC->Destroy(); }
	ActiveNPCs.Empty();

	TArray<EShopCategory> SelectedCategories = SelectShopCategories(ShopSpots.Num());
	TArray<APTWShopSpot*> ShuffledSpots = ShopSpots;

	int32 LastIndex = ShuffledSpots.Num() - 1;
	for (int32 i = 0; i <= LastIndex; ++i)
	{
		int32 Index = FMath::RandRange(i, LastIndex);
		ShuffledSpots.Swap(i, Index);
	}

	int32 SpawnCount = FMath::Min(SelectedCategories.Num(), ShuffledSpots.Num());
	for (int32 i = 0; i < SpawnCount; ++i)
	{
		APTWShopSpot* Spot = ShuffledSpots[i];
		EShopCategory Category = SelectedCategories[i];

		FActorSpawnParameters Params;
		APTWShopNPC* NewNPC = GetWorld()->SpawnActor<APTWShopNPC>(ShopNPCClass, Spot->GetActorTransform(), Params);

		if (NewNPC)
		{
			bool bIsGacha = GetCurrentRoundEventTag().MatchesTag(GameplayTags::Event::Round::Shop::Gacha);

			TArray<FName> ShopItems = SelectItemsForShop(Category, FGameplayTag());

			if (GetCurrentRoundEventTag().MatchesTag(GameplayTags::Event::Round::Shop::OpenRun))
			{
				for (FName Item : ShopItems)
				{
					if (!GlobalItemStock.Contains(Item)) GlobalItemStock.Add(Item, 2);
				}
			}

			NewNPC->InitializeShop(Category, ShopItems, Spot->GetItemSpawnTransforms(), bIsGacha);
			ActiveNPCs.Add(NewNPC);
			NewNPC->SetAssignedSpot(Spot);
		}
	}

	if (GetCurrentRoundEventTag().MatchesTag(GameplayTags::Event::Round::Shop::FlashSale))
	{
		if (UWorld* World = GetWorld())
		{
			for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
			{
				if (APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get()))
				{
					FText MessageText = FText::FromString(TEXT("상점이 변경되었습니다!"));
					PC->SendMessage(MessageText, ENotificationPriority::Normal, 2.0f, false);
				}
			}
		}
		
		float RandomDelay = FMath::RandRange(3.0f, 10.0f);
		GetWorld()->GetTimerManager().SetTimer(FlashSaleTimerHandle, this, &UPTWShopSubsystem::ReShuffleShops, RandomDelay, false);
	}
}

TArray<EShopCategory> UPTWShopSubsystem::SelectShopCategories(int32 TargetCount)
{
	TArray<EShopCategory> Result;
	TArray<EShopCategory> Pool;

	if (GetCurrentRoundEventTag().MatchesTag(GameplayTags::Event::Round::Shop::ExtremeDilemma))
	{
		Pool = { EShopCategory::Attack, EShopCategory::Lobby };
	}
	else
	{
		Pool = { EShopCategory::Attack, EShopCategory::Defense, EShopCategory::Utility, EShopCategory::Chaos, EShopCategory::Lobby };
	}

	if (TargetCount <= 0) return Result;

	for (EShopCategory Type : Pool)
	{
		if (Result.Num() < TargetCount) Result.Add(Type);
	}

	while (Result.Num() < TargetCount)
	{
		int32 RandIdx = FMath::RandRange(0, Pool.Num() - 1);
		Result.Add(Pool[RandIdx]);
	}

	return Result;
}

TArray<FName> UPTWShopSubsystem::SelectItemsForShop(EShopCategory Category, FGameplayTag BanTag)
{
	TArray<FName> Candidates;

	for (const auto& Elem : CachedShopItems)
	{
		const FShopItemRow& Row = Elem.Value;

		if (Row.Category == Category)
		{
			if (!BanTag.IsValid() || !Row.BannedGameModes.HasTag(BanTag))
			{
				Candidates.Add(Elem.Key);
			}
		}
	}

	TArray<FName> SelectedItems;
	int32 RequiredCount = 3;

	while (SelectedItems.Num() < RequiredCount && Candidates.Num() > 0)
	{
		int32 Idx = FMath::RandRange(0, Candidates.Num() - 1);
		SelectedItems.Add(Candidates[Idx]);
		Candidates.RemoveAt(Idx);
	}

	return SelectedItems;
}

const FShopItemRow* UPTWShopSubsystem::GetShopItemData(FName ItemID) const
{
	return CachedShopItems.Find(ItemID);
}

bool UPTWShopSubsystem::CanAffordItem(APTWPlayerState* BuyerPS, int32 Price) const
{
	if (!BuyerPS) return false;

	if (GetCurrentRoundEventTag().MatchesTag(GameplayTags::Event::Round::Shop::Debt))
	{
		return true;
	}

	return true;
}

bool UPTWShopSubsystem::TryPurchaseItem(APTWPlayerState* BuyerPS, APTWShopNPC* TargetNPC, FName ItemID)
{
	if (!BuyerPS || !TargetNPC) return false;

	UE_LOG(LogTemp, Error, TEXT("[ShopSubsystem] 아이템 구매 시도!"));

	if (GetCurrentRoundEventTag().MatchesTag(GameplayTags::Event::Round::Shop::OpenRun))
	{
		int32& CurrentCount = ShopPurchaseCounts.FindOrAdd(TargetNPC);

		if (CurrentCount >= 2)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ShopEvent] 이 상점은 이미 매진되어 사라졌습니다."));
			return false;
		}

		CurrentCount++;

		if (CurrentCount >= 2)
		{
			UE_LOG(LogTemp, Error, TEXT("[ShopEvent] 상점 이용 제한 도달! 상점을 철수합니다."));

			ActiveNPCs.Remove(TargetNPC);
			TargetNPC->Destroy();
		}
	}

	if (GetCurrentRoundEventTag().MatchesTag(GameplayTags::Event::Round::Shop::FlashSale))
	{
		if (APTWShopSpot* Spot = TargetNPC->GetAssignedSpot())
		{
			BuyerPS->Client_AddDepletedSpot(Spot);
		}
	}

	if (GetCurrentRoundEventTag().MatchesTag(GameplayTags::Event::Round::Shop::RiskPurchase))
	{

	}

	return true;
}

void UPTWShopSubsystem::AddCrowdFunding(int32 Amount, APTWPlayerState* Donator)
{
	if (bIsShopOpenedByFunding) return;

	CurrentCrowdFundingAmount += Amount;

	if (CurrentCrowdFundingAmount >= 1000)
	{
		bIsShopOpenedByFunding = true;
		GetWorld()->GetTimerManager().ClearTimer(CrowdFundingTimerHandle);

		UE_LOG(LogTemp, Warning, TEXT("[ShopEvent] 펀딩 1000G 달성! 상점 강제 오픈!"));
		ReShuffleShops();
	}
}

void UPTWShopSubsystem::OnCrowdFundingFailed()
{
	if (!bIsShopOpenedByFunding)
	{
		UE_LOG(LogTemp, Error, TEXT("[ShopEvent] 펀딩 실패! 모인 재화(%d)는 증발하며 상점은 열리지 않습니다."), CurrentCrowdFundingAmount);
		CurrentCrowdFundingAmount = 0;
	}
}

FGameplayTag UPTWShopSubsystem::GetCurrentRoundEventTag() const
{
	if (UWorld* World = GetWorld())
	{
		if (APTWGameState* GS = World->GetGameState<APTWGameState>())
		{
			return GS->GetCurrentRoundEventTag();
		}
	}
	return FGameplayTag::EmptyTag;
}
