// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWCollapseManager.generated.h"

class APTWFloorTile;
class AGroupActor;

USTRUCT(BlueprintType)
struct FFloorLayer
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<APTWFloorTile*> Tiles;
};

UCLASS()
class PTW_API APTWCollapseManager : public AActor
{
	GENERATED_BODY()

public:
	APTWCollapseManager();
	void ExecuteCollapseSequence(int32 TileIndex);
	FORCEINLINE int32 GetTileGroupParentsSize() const { return TileGroupParents.Num(); }

protected:
	virtual void BeginPlay() override;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<AActor>> TileGroupParents;
	
private:
	FTimerHandle CollapseTimerHandle;
	int32 CurrentIndex = 0;
	int32 MaxIndex;
};
