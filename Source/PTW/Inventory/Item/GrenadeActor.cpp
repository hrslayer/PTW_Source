#include "GrenadeActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "PTW.h"
#include "Components/BoxComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PTWGameplayTag/GameplayTags.h"


AGrenadeActor::AGrenadeActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(FName("BoxComponent"));
	SetRootComponent(BoxComponent);
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(BoxComponent);
	
	BoxComponent->OnComponentHit.AddDynamic(this, &AGrenadeActor::OnHit);
}

void AGrenadeActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (Hit.Normal.Z > 0.7f)
	{
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->Deactivate();
		SetActorRotation(Hit.Normal.Rotation());
		
		Explode();
	}
}

void AGrenadeActor::Explode()
{
	 if (!HasAuthority()) return;
	
	TArray<FOverlapResult> OverlapResults;
	FVector ExplodeLoc = GetActorLocation();
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	
	bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		ExplodeLoc,
		FQuat::Identity,
		ECC_WeaponAttack,
		FCollisionShape::MakeSphere(ExplodeRadius),
		Params
	);
	
	
	if (bHasOverlap)
	{
		for (auto& Result : OverlapResults)
		{
			AActor* Victim = Result.GetActor();
			if (Victim)
			{
				ApplyExplosionDamage(Victim);
			}
		}
	}
	
	AActor* Shooter = GetInstigator();
	UAbilitySystemComponent* InstigatorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Shooter);
	
	FGameplayCueParameters CueParams;
	CueParams.Location = GetActorLocation();
	CueParams.Instigator = Shooter;
	InstigatorASC->ExecuteGameplayCue(GameplayTags::GameplayCue::Weapon::Explosion, CueParams);
	
	Destroy();
}

void AGrenadeActor::ApplyExplosionDamage(AActor* TargetActor)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;
	
	FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageGEClass, 1.0f, TargetASC->MakeEffectContext());
	
	if (SpecHandle.IsValid())
	{
		float Distance = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
		SpecHandle.Data->SetSetByCallerMagnitude(GameplayTags::Data::Damage, -BaseDamage);
		SpecHandle.Data->SetSetByCallerMagnitude(GameplayTags::Data::Distance, Distance);
		SpecHandle.Data->SetSetByCallerMagnitude(GameplayTags::Data::Radius, ExplodeRadius);
		TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}




