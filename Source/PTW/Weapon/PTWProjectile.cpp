#include "PTWProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "PTW.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "GAS/PTWWeaponAttributeSet.h"
#include "PTWGameplayTag/GameplayTags.h"

APTWProjectile::APTWProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	SetRootComponent(CapsuleComponent);
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CapsuleComponent);
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	
	CapsuleComponent->OnComponentHit.AddDynamic(this, &APTWProjectile::OnHit);
	ProjectileMovementComponent->UpdatedComponent = CapsuleComponent;
	
	InitialLifeSpan = 3.0f; 
}

void APTWProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void APTWProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
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
	
	float FinalDamage = InstigatorASC->GetNumericAttributeChecked(UPTWWeaponAttributeSet::GetDamageAttribute());
	
	TArray<FOverlapResult> OverlapResults;
	if (ExplosionOverlapSetter(OverlapResults))
	{
		ApplyExplosionDamage(OverlapResults, FinalDamage);
	}
	
	FGameplayCueParameters CueParams;
	CueParams.Location = GetActorLocation();
	CueParams.Instigator = Shooter;
	InstigatorASC->ExecuteGameplayCue(GameplayTags::GameplayCue::Weapon::Explosion, CueParams);
	
	Destroy();
}

bool APTWProjectile::ExplosionOverlapSetter(TArray<FOverlapResult>& OverlapResults)
{
	FVector ExplosionLocation = GetActorLocation();
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(ExplosionRad);
	FCollisionQueryParams CollisionParams;
			
	if (!bIsSelfTarget)
	{
		CollisionParams.AddIgnoredActor(this);
		CollisionParams.AddIgnoredActor(GetInstigator());
	}
	
	bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		ExplosionLocation,
		FQuat::Identity,
		ECC_WeaponAttack,
		SphereShape,
		CollisionParams
		);
				
	DrawDebugSphere(GetWorld(), ExplosionLocation, ExplosionRad, 32, FColor::Red, false, 2.0f);
	
	return bHasOverlap;
}

void APTWProjectile::ApplyExplosionDamage(TArray<FOverlapResult>& OverlapResults, float FinalDamage)
{
	TSet<AActor*> ProcessedActors;
	const FVector ExplosionLocation = GetActorLocation();
	const float ExplosionRadius = ExplosionRad; // 프로젝타일에 설정된 반경 값

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* HitActor = Result.GetActor();
		if (!HitActor || ProcessedActors.Contains(HitActor)) continue;
		ProcessedActors.Add(HitActor);

		FHitResult ObstacleHit;
		if (CheckingBlock(ObstacleHit, ExplosionLocation, HitActor))
		{
			if (ObstacleHit.GetActor() != HitActor) continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (TargetASC && DamageSpecHandle.IsValid())
		{
			FGameplayEffectSpec* NewSpec = new FGameplayEffectSpec(*DamageSpecHandle.Data.Get());
			FGameplayEffectSpecHandle IndividualHandle(NewSpec);
			
			float Distance = FVector::Dist(ExplosionLocation, HitActor->GetActorLocation());
			
			IndividualHandle.Data->SetSetByCallerMagnitude(GameplayTags::Data::Damage, -FinalDamage);
			IndividualHandle.Data->SetSetByCallerMagnitude(GameplayTags::Data::Distance, Distance);
			IndividualHandle.Data->SetSetByCallerMagnitude(GameplayTags::Data::Radius, ExplosionRadius);


			TargetASC->ApplyGameplayEffectSpecToSelf(*IndividualHandle.Data.Get());
			
			FGameplayCueParameters TargetCueParams;
			TargetCueParams.Location = HitActor->GetActorLocation();
			TargetCueParams.Instigator = GetInstigator();
			TargetASC->ExecuteGameplayCue(GameplayTags::GameplayCue::Weapon::HitImpact, TargetCueParams);
		}
	}
}

bool APTWProjectile::CheckingBlock(FHitResult& ObstarcleHit, const FVector ExplosionLocation, const AActor* HitActor)
{
	if (!HitActor) return false;
	
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.AddIgnoredActor(GetInstigator());
	
	FVector TargetPoint = HitActor->GetActorLocation() + FVector(0.f, 0.f, 40.f);
	
	return GetWorld()->LineTraceSingleByChannel(
		ObstarcleHit,
		ExplosionLocation,
		TargetPoint,
		ECC_Visibility,
		CollisionParams
	);
}

