// Fill out your copyright notice in the Description page of Project Settings.


#include "Event/ChaosEvent/PTWChaosEvent_IceFloor.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/Character/Component/PTWChaosAffectedComponent.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "GameFramework/PlayerState.h"

void UPTWChaosEvent_IceFloor::ApplyEvent(APTWGameState* GameState)
{
	if (!GameState) return;
	
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		APTWPlayerCharacter* Character = Cast<APTWPlayerCharacter>(PlayerState->GetPawn());
		if (!Character) continue;
		
		UPTWChaosAffectedComponent* ChaosAffectedComponent = Character->GetChaosAffectedComponent();
		if (!ChaosAffectedComponent) continue;

		ChaosAffectedComponent->Multicast_SetIceFloorEvent(true);
	}
	
}

void UPTWChaosEvent_IceFloor::EndEvent(APTWGameState* GameState)
{
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		APTWPlayerCharacter* Character = Cast<APTWPlayerCharacter>(PlayerState->GetPawn());
		if (!Character) continue;
		
		UPTWChaosAffectedComponent* ChaosAffectedComponent = Character->GetChaosAffectedComponent();
		if (!ChaosAffectedComponent) continue;

		ChaosAffectedComponent->Multicast_SetIceFloorEvent(false);
	}
}
