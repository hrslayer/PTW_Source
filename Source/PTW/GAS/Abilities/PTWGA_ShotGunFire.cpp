// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_ShotGunFire.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "PTW.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "Inventory/Instance/PTWWeaponInstance.h"
#include "PTWGameplayTag/GameplayTags.h"


void UPTWGA_ShotGunFire::HandleHitScan(const FPTWFireConext Context)
{
	TArray<FHitResult> HitResults;
	TArray<AActor*> HitActors;
	PerformLineTraceShotGun(HitResults, Context.PC);
	
	if (!HasAuthority(&CurrentActivationInfo)) return;
	
	for (FHitResult& Hit : HitResults)
	{
		if (ValidateHitResult(Hit))
		{
			FGameplayAbilityTargetDataHandle TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit);
			
			float Damage = CalculateDamage(Context);
			
			ApplyDamageToTarget(TargetData, Damage);
			
			if (!HitActors.Contains(Hit.GetActor()))
			{
				ExecuteHitImpactCue(Hit);
				HitActors.Add(Hit.GetActor());
			}
		}
	}
}

void UPTWGA_ShotGunFire::ApplyDamageToTarget(const FGameplayAbilityTargetDataHandle& TargetData, float BaseDamage)
{
	TMap<AActor*, int32> HitCounts;
	
	for (auto Data : TargetData.Data)
	{
		AActor* HitActor = Data->GetActors()[0].Get();
		if (HitActor == GetAvatarActorFromActorInfo()) continue;
		
		if (HitActor)
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
			if (!TargetASC) continue;
			
			if (!CheckingTag(TargetASC) && CheckingTeam(HitActor))
			{
				const FHitResult* HitResult = Data->GetHitResult();
				if (HitResult->bBlockingHit)
				{
					HitAction(HitResult);
				}
				HitCounts.FindOrAdd(HitActor)++;
			}
		}
	}
	
	for (auto& Pair : HitCounts)
	{
		AActor* TargetActor = Pair.Key;
		int32 HitCount = Pair.Value;
		float FinalDamage = BaseDamage * HitCount;
		
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageGEClass, GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(GameplayTags::Data::Damage, -FinalDamage);
			
			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
			{
				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}

void UPTWGA_ShotGunFire::PerformLineTraceShotGun(TArray<FHitResult>& OutHitResult, APTWPlayerCharacter* PlayerCharacter)
{
	const int32 PelletCount = 8;
	const float SpreadAngle = 5.0f;
	const float SweepRad = 5.0f;
	
	APTWPlayerController* Controller = PlayerCharacter->GetController<APTWPlayerController>();
	if (!Controller) return;
	
	FVector StartLoc;
	FRotator ViewRot;
	Controller->GetPlayerViewPoint(StartLoc, ViewRot);
	
	AActor* Avatar = GetAvatarActorFromActorInfo();
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Avatar); 
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(SweepRad);
	
	for (int32 i = 0; i < PelletCount; ++i)
	{
		float RandomPitch = FMath::FRandRange(-SpreadAngle, SpreadAngle);
		float RandomYaw = FMath::FRandRange(-SpreadAngle, SpreadAngle);
        
		FRotator SpreadRot = ViewRot;
		SpreadRot.Pitch += RandomPitch;
		SpreadRot.Yaw += RandomYaw;
        
		FVector End = StartLoc + (SpreadRot.Vector() * MaxRange);
		
		FHitResult IndividualHit;
		bool bHit = GetWorld()->SweepSingleByChannel(
			IndividualHit,
			StartLoc,
			End,
			FQuat::Identity,
			ECC_WeaponAttack,
			SphereShape,
			Params
			);
		
		// 디버깅 로직 시작 
		FColor LineColor = bHit ? FColor::Red : FColor::Green; 
		float LifeTime = 2.0f; 
		
		DrawDebugLine(GetWorld(), StartLoc, End, LineColor, false, LifeTime, 0, 1.5f);
		
		if (bHit)
		{
			DrawDebugPoint(GetWorld(), IndividualHit.ImpactPoint, 10.0f, FColor::Yellow, false, LifeTime);
			// 디버깅 로직 종료 
			OutHitResult.Add(IndividualHit);
		}
	}
}

void UPTWGA_ShotGunFire::HitAction(const FHitResult* HitResult)
{
	UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
	if (MyASC)
	{
		FGameplayCueParameters CueParams;
		CueParams.Instigator = GetAvatarActorFromActorInfo();
		CueParams.Location = HitResult->ImpactPoint;

		if (HitResult->BoneName == FName("head"))
		{
			CueParams.AggregatedSourceTags.AddTag(
				GameplayTags::State::HitReaction_HeadShot
			);
		}

		MyASC->ExecuteGameplayCue(
						GameplayTags::GameplayCue::Hit::Confirm,
						CueParams
					);
	}
}
	
