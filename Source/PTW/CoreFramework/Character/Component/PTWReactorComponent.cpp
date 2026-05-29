// Fill out your copyright notice in the Description page of Project Settings.

#include "CoreFramework/Character/Component/PTWReactorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemGlobals.h"

UPTWReactorComponent::UPTWReactorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPTWReactorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{

		}
	}
}

void UPTWReactorComponent::ProcessDeath()
{
	Multicast_Death();
}

void UPTWReactorComponent::SetOutlineStencil(int32 StencilValue)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	if (ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor))
	{
		if (USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh())
		{
			MeshComp->SetRenderCustomDepth(StencilValue > 0);
			MeshComp->SetCustomDepthStencilValue(StencilValue);
		}
	}
}

void UPTWReactorComponent::ClearOutlineStencil()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;
	
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor))
	{
		if (USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh())
		{
			MeshComp->SetRenderCustomDepth(false);
			MeshComp->SetCustomDepthStencilValue(0);
		}
	}
}

void UPTWReactorComponent::Multicast_Death_Implementation()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	if (UCapsuleComponent* Capsule = OwnerCharacter->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	if (UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
		Movement->SetComponentTickEnabled(false);
	}

	if (USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
	{
		Mesh->SetCollisionProfileName(TEXT("Ragdoll"));
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetAllBodiesSimulatePhysics(true);
		Mesh->SetSimulatePhysics(true);
		Mesh->WakeAllRigidBodies();
		Mesh->bPauseAnims = true;
	}

	OwnerCharacter->SetLifeSpan(3.0f);
}

void UPTWReactorComponent::Multicast_PlayHitReact_Implementation(const FVector& ImpactPoint)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	FVector HitVector = (ImpactPoint - OwnerCharacter->GetActorLocation()).GetSafeNormal();
	FRotator HitLocalRot = UKismetMathLibrary::InverseTransformDirection(OwnerCharacter->GetActorTransform(), HitVector).Rotation();
	float HitYaw = HitLocalRot.Yaw;

	UAnimMontage* MontageToPlay = nullptr;

	if (HitYaw >= -45.f && HitYaw <= 45.f) MontageToPlay = HitReact_Front;
	else if (HitYaw >= -135.f && HitYaw < -45.f) MontageToPlay = HitReact_Left;
	else if (HitYaw > 45.f && HitYaw <= 135.f) MontageToPlay = HitReact_Right;
	else MontageToPlay = HitReact_Back;

	if (MontageToPlay && OwnerCharacter->GetMesh() && OwnerCharacter->GetMesh()->GetAnimInstance())
	{
		OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Play(MontageToPlay, 1.0f);
	}
}

void UPTWReactorComponent::OnTagChanged(const FGameplayTag Tag, int32 NewCount)
{

}

void UPTWReactorComponent::OnInputTriggered()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(IdleCheckTimerHandle);
	}

	if (bIsIdleState)
	{
		SetIdleState(false);
	}
}

void UPTWReactorComponent::OnInputCompleted()
{
	UWorld* World = GetWorld();
	if (World && !World->GetTimerManager().IsTimerActive(IdleCheckTimerHandle))
	{
		World->GetTimerManager().SetTimer(IdleCheckTimerHandle, this, &UPTWReactorComponent::CheckIdleCondition, 0.1f, true);
	}
}

void UPTWReactorComponent::CheckIdleCondition()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	bool bIsStationary = OwnerCharacter->GetVelocity().SizeSquared() < 10.0f;
	bool bIsNotFalling = false;

	if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
	{
		bIsNotFalling = !MovementComp->IsFalling();
	}
	if (bIsStationary && bIsNotFalling)
	{
		SetIdleState(true);

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(IdleCheckTimerHandle);
		}
	}
}

void UPTWReactorComponent::SetIdleState(bool bNewState)
{
	if (bIsIdleState == bNewState) return;

	bIsIdleState = bNewState;

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerActor);
	if (!ASC) return;

	if (bIsIdleState)
	{
		ASC->AddLooseGameplayTag(IdleGameplayTag);
	}
	else
	{
		ASC->RemoveLooseGameplayTag(IdleGameplayTag);
	}
}
