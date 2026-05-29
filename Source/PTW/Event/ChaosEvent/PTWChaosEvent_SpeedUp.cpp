// Fill out your copyright notice in the Description page of Project Settings.


#include "Event/ChaosEvent/PTWChaosEvent_SpeedUp.h"

#include "Kismet/GameplayStatics.h"

void UPTWChaosEvent_SpeedUp::ApplyEvent(APTWGameState* GameState)
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 3.f);
}

void UPTWChaosEvent_SpeedUp::EndEvent(APTWGameState* GameState)
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
}
