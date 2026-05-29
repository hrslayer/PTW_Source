// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/PTWGA_Vacuum.h"
#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "Inventory/Instance/PTWWeaponInstance.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "Weapon/PTWVacuumWeaponData.h"
#include "Engine/OverlapResult.h"
#include "CoreFramework/Character/Component/PTWWeaponComponent.h"

UPTWGA_Vacuum::UPTWGA_Vacuum()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 과열 상태일 때는 이 능력이 발동되는 것을 차단
	ActivationBlockedTags.AddTag(GameplayTags::Weapon::State::Overheated);
}

void UPTWGA_Vacuum::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (RegenEffectHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveActiveGameplayEffect(RegenEffectHandle);
		}
	}
}

void UPTWGA_Vacuum::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	// 마우스를 떼어 흡입 기능이 끝나면 스태미나 충전 타이머를 시작
	if (RegenGameplayEffectClass)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(RegenGameplayEffectClass, GetAbilityLevel());
			if (SpecHandle.IsValid())
			{
				RegenEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}

void UPTWGA_Vacuum::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	StopFire();

	bool bReplicateEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPTWGA_Vacuum::StartFire()
{
	const FPTWFireConext Context = GetFireContext();
	if (!Context.IsValid()) return;

	FireRate = 0.05f; 

	// 첫 발 트리거
	AutoFire();

	if (!GetWorld()->GetTimerManager().IsTimerActive(AutoFireTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(AutoFireTimer, this, &UPTWGA_Vacuum::AutoFire, FireRate, true);
	}
}

void UPTWGA_Vacuum::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(AutoFireTimer);
}

void UPTWGA_Vacuum::AutoFire()
{/*
	UPTWWeaponInstance* WeaponInst = GetFireContext().WeaponInst;
	if (!WeaponInst) return;

	const UPTWVacuumWeaponData* VacuumData = Cast<UPTWVacuumWeaponData>(WeaponInst->GetWeaponData());
	if (!VacuumData) return;

	// 기존 탄약 속성을 스태미나 게이지로 판단하여 차감 처리
	if (!CommitAbilityCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		// 비용 지불 실패 시 과열(락다운) 처리
		StartOverheat(VacuumData->OverheatDuration);
		return;
	}


	// 물리적 흡입 연산 수행
	ApplySuction(VacuumData);
	*/

	const FPTWFireConext Context = GetFireContext();
	if (!Context.IsValid())
	{
		StopFire();
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 청소기 데이터 자산 가져오기 (주석 처리했던 원본 로직 유지 보정)
	const UPTWVacuumWeaponData* VacuumData = Cast<UPTWVacuumWeaponData>(Context.WeaponInst->GetWeaponData());
	if (!VacuumData) return;

	// ★ [핵심 우회] 팀원이 가로막은 부모의 깐깐한 조건문 분기를 무시하고, 
	// GAS 표준 함수인 CheckCost로 스태미나 잔여량을 매 프레임 선제 검사합니다.
	if (!CheckCost(CurrentSpecHandle, CurrentActorInfo))
	{
		// 스태미나가 아예 없거나 부족하다면 기획하신 '과열 락다운(5초)' 상태로 전환합니다.
		StopFire();
		StartOverheat(VacuumData->OverheatDuration);
		return;
	}

	// ★ [수정] 스태미나가 충분하므로 비용을 강제 확정 소모시킵니다. 
	// (ApplyAbilityCost 대신 GAS 엔진 내장 함수인 CommitAbilityCost를 호출하는 것이 정석입니다.)
	CommitAbilityCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);

	// 질문자님이 정교하게 구현해 두신 물리적 흡입 연산 수행
	ApplySuction(VacuumData);

	// 반동 적용 (헤더 추가로 이제 C2027 에러가 완벽하게 사라집니다)
	if (Context.PC && Context.PC->GetWeaponComponent())
	{
		Context.PC->GetWeaponComponent()->ApplyRecoil();
	}
}

void UPTWGA_Vacuum::ApplySuction(const UPTWVacuumWeaponData* VacuumData)
{
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PC) return;

	FVector StartLoc = PC->GetActorLocation();
	FVector ForwardVec = PC->GetActorForwardVector();

	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(VacuumData->VacuumRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PC);

	// ECC_PhysicsBody 채널을 가진 주변 오브젝트들을 검출
	if (GetWorld()->OverlapMultiByChannel(Overlaps, StartLoc, FQuat::Identity, ECC_PhysicsBody, Sphere, Params))
	{
		for (const FOverlapResult& Result : Overlaps)
		{
			AActor* TargetActor = Result.GetActor();
			if (!TargetActor) continue;

			UPrimitiveComponent* Mesh = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent());
			if (Mesh && Mesh->IsSimulatingPhysics())
			{
				// 정면 부채꼴 내 각도 검사 (내적 연산)
				FVector DirToTarget = (TargetActor->GetActorLocation() - StartLoc).GetSafeNormal();
				float DotProduct = FVector::DotProduct(ForwardVec, DirToTarget);

				if (DotProduct > FMath::Cos(FMath::DegreesToRadians(VacuumData->VacuumAngle)))
				{
					// 거리가 가까울수록 더 강하게 플레이어 쪽으로 잡아당기는 뺄셈 벡터 물리력 적용
					float Distance = FVector::Dist(StartLoc, TargetActor->GetActorLocation());
					float DistanceFactor = FMath::Clamp(1.0f - (Distance / VacuumData->VacuumRadius), 0.1f, 1.0f);

					FVector SuctionForce = -DirToTarget * VacuumData->SuctionStrength * DistanceFactor * Mesh->GetMass();
					Mesh->AddForce(SuctionForce);
				}
			}
		}
	}
}

void UPTWGA_Vacuum::StartOverheat(float Duration)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	// 네이티브로 등록한 과열 태그를 캐릭터 스태이트에 수동 부여
	FGameplayTag OverheatTag = GameplayTags::Weapon::State::Overheated;
	ASC->AddLooseGameplayTag(OverheatTag);

	UE_LOG(LogTemp, Warning, TEXT("청소기 과열 주의! 5초간 락다운 됩니다."));

	// 과열되었으므로 현재 활성화된 청소기 능력을 강제로 종료
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

	// 지정된 시간 뒤 과열 상태를 해제해 주는 타이머 구동
	GetWorld()->GetTimerManager().SetTimer(OverheatTimerHandle, [ASC, OverheatTag]()
		{
			if (ASC)
			{
				ASC->RemoveLooseGameplayTag(OverheatTag);
				UE_LOG(LogTemp, Log, TEXT("청소기 열이 식었습니다. 다시 가동 가능합니다."));
			}
		}, Duration, false);
}
