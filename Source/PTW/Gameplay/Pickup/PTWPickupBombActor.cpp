// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Pickup/PTWPickupBombActor.h"
#include "Components/SphereComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "PTW.h"

APTWPickupBombActor::APTWPickupBombActor()
{
	if (SphereComp)
	{
		SphereComp->SetSphereRadius(60.0f);
	}
}

void APTWPickupBombActor::OnPickedUp(APTWPlayerCharacter* Player)
{
	FVector ExplodeLocation = GetActorLocation();
	TArray<FOverlapResult> OverlapResults;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// ECC_WeaponAttack 채널로 주변 캐릭터 감지
	bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
		OverlapResults, ExplodeLocation, FQuat::Identity, ECC_WeaponAttack,
		FCollisionShape::MakeSphere(ExplosionRadius), QueryParams
	);

	TArray<APTWPlayerCharacter*> TargetsToAffect;
	if (Player) TargetsToAffect.Add(Player); // 청소기로 흡입한 사람 무조건 포함

	if (bHasOverlap)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			if (APTWPlayerCharacter* HitPlayer = Cast<APTWPlayerCharacter>(Result.GetActor()))
			{
				if (!TargetsToAffect.Contains(HitPlayer)) TargetsToAffect.Add(HitPlayer);
			}
		}
	}

	// 수집된 모든 타겟에게 데미지(GAS) 및 넉백(Launch) 적용
	for (APTWPlayerCharacter* TargetPlayer : TargetsToAffect)
	{
		if (!TargetPlayer) continue;

		UAbilitySystemComponent* TargetASC = TargetPlayer->GetAbilitySystemComponent();

		// GAS 데미지 적용
		if (TargetASC && BombDamageGEClass)
		{
			FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
			EffectContext.AddInstigator(TargetPlayer, this);

			FHitResult CustomHit;
			CustomHit.bBlockingHit = true;
			CustomHit.ImpactPoint = ExplodeLocation;
			CustomHit.TraceStart = ExplodeLocation - FVector(0.f, 0.f, 20.f);
			CustomHit.TraceEnd = ExplodeLocation;
			EffectContext.AddHitResult(CustomHit);

			FGameplayEffectSpecHandle DamageSpecHandle = TargetASC->MakeOutgoingSpec(BombDamageGEClass, 1.0f, EffectContext);
			if (DamageSpecHandle.IsValid())
			{
				float Distance = FVector::Dist(ExplodeLocation, TargetPlayer->GetActorLocation());

				// 팀원 MMC가 거리 감쇄를 정상 연산할 수 있도록 데이터 주입
				DamageSpecHandle.Data->SetSetByCallerMagnitude(GameplayTags::Data::Damage, -BombDamage);
				DamageSpecHandle.Data->SetSetByCallerMagnitude(GameplayTags::Data::Distance, Distance);
				DamageSpecHandle.Data->SetSetByCallerMagnitude(GameplayTags::Data::Radius, ExplosionRadius);

				TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
			}
		}

		// 넉백 로직
		// 폭발 중심점(ExplodeLocation)으로부터 캐릭터를 바깥으로 밀어냅니다.
		FVector LaunchDir = TargetPlayer->GetActorLocation() - ExplodeLocation;
		LaunchDir.Z = 0.0f; // 수평 밀치기를 위해 Z축 기본값은 제거
		LaunchDir.Normalize();

		float Distance = FVector::Dist(ExplodeLocation, TargetPlayer->GetActorLocation());

		// 중심에서 멀어질수록 힘이 약해지는 비율 계산 (최소 0.2배 보장)
		float Falloff = FMath::Clamp(1.0f - (Distance / ExplosionRadius), 0.2f, 1.0f);

		FVector FinalLaunch = (LaunchDir * (BombLaunchStrength * Falloff)) + FVector(0.f, 0.f, BombUpwardForce * Falloff);

		// 엔지 기본 물리 함수로 타겟을 즉시 날려버립니다 (자해 방지 등 모든 필터 우회)
		TargetPlayer->LaunchCharacter(FinalLaunch, true, true);

		// 연출용 GameplayCue 및 과열 태그 적용 ---
		if (TargetASC)
		{
			// 폭발 이펙트 큐 실행
			//TargetASC->AddGameplayCue(GameplayTags::GameplayCue::Bomb::Explode, TargetASC->MakeEffectContext());

			// 과열 태그 적용
			if (BombOverheatGEClass)
			{
				FGameplayEffectSpecHandle OverheatSpecHandle = TargetASC->MakeOutgoingSpec(BombOverheatGEClass, 1.0f, TargetASC->MakeEffectContext());
				if (OverheatSpecHandle.IsValid())
				{
					TargetASC->ApplyGameplayEffectSpecToSelf(*OverheatSpecHandle.Data.Get());
				}
			}
		}
	}
}
