// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWKnockBackProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"


APTWKnockBackProjectile::APTWKnockBackProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APTWKnockBackProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void APTWKnockBackProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || !OtherActor || OtherActor == GetInstigator()) return;
	
	AActor* Shooter = GetInstigator();
	UAbilitySystemComponent* InstigatorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Shooter);
	
	if (!InstigatorASC)
	{
		Destroy();
		return;
	}
	
	TArray<FOverlapResult> OverlapResults;
	if (ExplosionOverlapSetter(OverlapResults))
	{
		ApplyKnockBack(OverlapResults);
	}
	
	Destroy();
}

void APTWKnockBackProjectile::ApplyKnockBack(TArray<FOverlapResult>& OverlapResults)
{
	TSet<AActor*> ProcessedActors;
	const FVector ExplosionLocation = GetActorLocation();
	const float ExplosionRadius = ExplosionRad; 
	
	AActor* BombInstigator = GetInstigator();
	
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* HitActor = OverlapResult.GetActor();
		if (!HitActor || ProcessedActors.Contains(HitActor)) continue;
		ProcessedActors.Add(HitActor);
		
		FHitResult ObstacleHit;
		if (CheckingBlock(ObstacleHit, ExplosionLocation, HitActor))
		{
			if (ObstacleHit.GetActor() != HitActor) continue;
		}
		
		ACharacter* Vic = Cast<ACharacter>(HitActor);
		if (Vic)
		{
			FVector FinalLaunch = FVector::ZeroVector;
			
			if (Vic == BombInstigator)
			{
				FinalLaunch = FVector(0.0f, 0.0f, MaxUpwardForce * 1.2f);
			}
			else
			{
				FVector LaunchDir = Vic->GetActorLocation() - GetActorLocation();
				LaunchDir.Z = 0.0f;
				LaunchDir.Normalize();
          
				float Distance = FVector::Dist(ExplosionLocation, Vic->GetActorLocation());
				float Falloff = FMath::Clamp(1.0f - (Distance / ExplosionRadius), 0.2f, 1.0f);
				
				FinalLaunch = (LaunchDir * MaxLaunchStrength * Falloff) + FVector(0, 0, MaxUpwardForce);
			}
			
			if (APTWPlayerCharacter* VictimChar = Cast<APTWPlayerCharacter>(Vic))
			{
				APTWPlayerCharacter* InstigatorChar = Cast<APTWPlayerCharacter>(BombInstigator);
				if (InstigatorChar && VictimChar != InstigatorChar)
				{
					VictimChar->SetLastAttacker(InstigatorChar->GetPlayerState());
				}
			}
			Vic->LaunchCharacter(FinalLaunch, true, true);
		}
	}
	
}

