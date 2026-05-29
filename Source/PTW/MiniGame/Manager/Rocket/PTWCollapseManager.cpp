#include "PTWCollapseManager.h"

#include "Kismet/GameplayStatics.h"
#include "MiniGame/Actor/Rocket/PTWFloorTile.h"


APTWCollapseManager::APTWCollapseManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void APTWCollapseManager::BeginPlay()
{
	Super::BeginPlay();
	MaxIndex = TileGroupParents.Num();
}

void APTWCollapseManager::ExecuteCollapseSequence(int32 TileIndex)
{
	if (CurrentIndex >= MaxIndex)
	{
		GetWorld()->GetTimerManager().ClearTimer(CollapseTimerHandle);
		return;
	}
	
	TArray<AActor*> AllDescendants;
	if (TileGroupParents.IsValidIndex(TileIndex))
	{
		TileGroupParents[TileIndex]->GetAttachedActors(AllDescendants, true);
		
		for (AActor* Actor : AllDescendants)
		{
			APTWFloorTile* TileActor = Cast<APTWFloorTile>(Actor);
			if (!TileActor) continue;
		
			TileActor->StartCollapse();
		}
		
		TileGroupParents.RemoveAt(TileIndex);
	}
}

