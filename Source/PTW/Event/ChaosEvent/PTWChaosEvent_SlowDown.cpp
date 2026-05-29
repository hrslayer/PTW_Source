// Fill out your copyright notice in the Description page of Project Settings.


#include "Event/ChaosEvent/PTWChaosEvent_SlowDown.h"

#include "Kismet/GameplayStatics.h"

void UPTWChaosEvent_SlowDown::ApplyEvent(APTWGameState* GameState)
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.75f);
}

void UPTWChaosEvent_SlowDown::EndEvent(APTWGameState* GameState)
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
}
