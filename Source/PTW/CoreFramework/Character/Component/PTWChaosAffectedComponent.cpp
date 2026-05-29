// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/Character/Component/PTWChaosAffectedComponent.h"

#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PTWGameplayTag/GameplayTags.h"

UPTWChaosAffectedComponent::UPTWChaosAffectedComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPTWChaosAffectedComponent::ApplyCurrentChaosEvent()
{
	APTWGameState* GameState = GetWorld()->GetGameState<APTWGameState>();
	if (!GameState) return;

	if (GameState->GetCurrentChaosEvent().HasTag(GameplayTags::Event::Chaos::IceFloor))
	{
		Multicast_SetIceFloorEvent(true);
	}
}

void UPTWChaosAffectedComponent::Client_SetInputState_Implementation(const bool bFrozen, const bool bInvert)
{
	bLookActionFrozen = bFrozen;
	bInputInvert = bInvert;
}

void UPTWChaosAffectedComponent::Multicast_SetIceFloorEvent_Implementation(bool bEnable)
{
	UCharacterMovementComponent* MovementComponent = GetOwnerMovement();
	if (!MovementComponent) return;

	if (bEnable)
	{
		CacheDefaultMovementValues();

		MovementComponent->GroundFriction = 0.f;
		MovementComponent->BrakingDecelerationWalking = 0.f;
	}
	else
	{
		MovementComponent->GroundFriction = DefaultGroundFriction;
		MovementComponent->BrakingDecelerationWalking = DefaultBrakingDeceleration;
	}
}


void UPTWChaosAffectedComponent::BeginPlay()
{
	Super::BeginPlay();
}

UCharacterMovementComponent* UPTWChaosAffectedComponent::GetOwnerMovement()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return nullptr;

	return Character->GetCharacterMovement();
}

void UPTWChaosAffectedComponent::CacheDefaultMovementValues()
{
	UCharacterMovementComponent* MovementComponent = GetOwnerMovement();
	if (!MovementComponent) return;

	DefaultGroundFriction = MovementComponent->GroundFriction;
	DefaultBrakingDeceleration = MovementComponent->BrakingDecelerationWalking;
}




