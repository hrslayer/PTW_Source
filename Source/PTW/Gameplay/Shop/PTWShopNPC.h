// Fill out your copyright notice in the Description page of Project Settings.

// Source/PTW/Gameplay/Shop/PTWShopNPC.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Shop/PTWShopItemData.h"
#include "PTWShopNPC.generated.h"

class APTWDisplayItem;

UCLASS()
class PTW_API APTWShopNPC : public AActor
{
	GENERATED_BODY()

public:
	APTWShopNPC();

	/* Subsystem에서 호출.카테고리와 판매할 아이템 ID 목록을 받음 */
	void InitializeShop(EShopCategory InCategory, const TArray<FName>& InItemIDs, const TArray<FTransform>& DisplayLocs, bool bInGachaMode = false);

	void CloseShop();

	void SetAssignedSpot(class APTWShopSpot* InSpot);
	class APTWShopSpot* GetAssignedSpot() const { return AssignedSpot; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void CheckShopAvailability();

	UFUNCTION()
	void OnRep_ShopCategory();

	void ApplyCategoryVisuals();

	UFUNCTION()
	void OnRep_GachaMode();

	UFUNCTION()
	void OnRep_AssignedSpot();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_ShopCategory, VisibleInstanceOnly)
	EShopCategory ShopCategory;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<APTWDisplayItem>> DisplayItems;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APTWDisplayItem> DisplayItemClass;
	UPROPERTY(EditDefaultsOnly, Category = "Visual|Config")
	TMap<EShopCategory, TObjectPtr<UMaterialInterface>> CategoryMaterialMap;
	UPROPERTY(ReplicatedUsing = OnRep_GachaMode, VisibleInstanceOnly, BlueprintReadOnly, Category = "Shop State")
	bool bIsGachaMode;
	UPROPERTY(EditDefaultsOnly, Category = "Visual|Config")
	TObjectPtr<UMaterialInterface> GachaMaterial;
	UPROPERTY(ReplicatedUsing = OnRep_AssignedSpot)
	TObjectPtr<class APTWShopSpot> AssignedSpot;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> DefaultSceneRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> NPCMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> StandMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> DecoMesh;

	bool bIsLocallyOpen;
};
