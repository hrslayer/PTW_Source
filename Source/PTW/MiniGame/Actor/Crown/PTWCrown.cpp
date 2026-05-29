#include "PTWCrown.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "MiniGame/GameMode/PTWCrownGameMode.h"
#include "Net/UnrealNetwork.h"


APTWCrown::APTWCrown()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>("RotatingMovementComponent");
	
	StaticMeshComponent->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.2f));
	StaticMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	
	SetReplicates(true);
	SetReplicatingMovement(true);
}

void APTWCrown::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, OwnerCharacter);
	DOREPLIFETIME(ThisClass, PreliminaryOwner);
}

void APTWCrown::SetPreliminaryOwner(APTWPlayerCharacter* NewOwner)
{
	PreliminaryOwner = NewOwner;
}

void APTWCrown::StartMovingToOwner_Implementation()
{
}

void APTWCrown::MoveToOwner(float Speed)
{
	if (HasAuthority())
	{
		if (!FMath::IsNearlyZero(Speed))
		{
			const FVector CurrentLocation = GetActorLocation();
			FVector DestLocation;
			if (USkeletalMeshComponent* TargetHead = PreliminaryOwner->GetMesh())
			{
				DestLocation = TargetHead->GetSocketLocation(CrownSocketName);
			}
			else
			{
				DestLocation = PreliminaryOwner->GetActorLocation();
			}
	
			const float DeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.0f;
	
			FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, DestLocation, DeltaSeconds, Speed);
			SetActorLocation(NewLocation);
		}
	}
}

bool APTWCrown::HasReachedOwner()
{
	const FVector CurrentLocation = GetActorLocation();
	
	FVector DestLocation;
	if (USkeletalMeshComponent* TargetHead = PreliminaryOwner->GetMesh())
	{
		DestLocation = TargetHead->GetSocketLocation(CrownSocketName);
	}
	else
	{
		DestLocation = PreliminaryOwner->GetActorLocation();
	}
	
	if (CurrentLocation.Equals(DestLocation, 0.1f))
	{
		return true;
	}
	
	return false;
}

void APTWCrown::AttachToOwnerHead()
{
	if (HasAuthority())
	{
		if (!IsValid(PreliminaryOwner)) return;

		USkeletalMeshComponent* TargetHead = PreliminaryOwner->GetMesh();
		if (!IsValid(TargetHead)) return;

		SetOwner(PreliminaryOwner);
		OwnerCharacter = PreliminaryOwner;
		PreliminaryOwner = nullptr;

		AttachToComponent(TargetHead, FAttachmentTransformRules::SnapToTargetNotIncludingScale, CrownSocketName);
		StaticMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));
	}
	OnAttachToOwnerHead.Broadcast();
}

void APTWCrown::DetachFromOwner()
{
	if (HasAuthority())
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		// StaticMeshComponent->SetRelativeLocation(FVector::ZeroVector);

		OwnerCharacter = nullptr;
		SetOwner(nullptr);
	}
	OnDetachFromOwner.Broadcast();
}

void APTWCrown::BeginPlay()
{
	Super::BeginPlay();
	
	if (APTWCrownGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<APTWCrownGameMode>() : nullptr)
	{
		GM->SetSpawnedCrown(this);
	}
}
