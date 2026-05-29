// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Grenade.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/Instance/PTWActiveItemInstance.h"
#include "Inventory/Item/GrenadeActor.h"

void UPTWGA_Grenade::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UPTWGA_Grenade::InitializeVariable()
{
	Super::InitializeVariable();
}

void UPTWGA_Grenade::ApplyItemEffect()
{
	if (!CommitCheck(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo)) return;
	
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PC) return;
	
	float BaseDamage = GetGrenadeBaseDamage(PC);
	
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);
	
	FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * 10000.0f);
	FHitResult ScreenHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PC);
	
	
	GetWorld()->LineTraceSingleByChannel(ScreenHit, CameraLocation, TraceEnd, ECC_Visibility, Params);
	
	FVector TargetLocation = ScreenHit.bBlockingHit ? ScreenHit.ImpactPoint : TraceEnd;
	FVector SpawnLocation = PC->GetActorLocation() + PC->GetActorForwardVector() * 50.f + PC->GetActorUpVector() * 50.f;
	FVector LaunchDirection = (TargetLocation - SpawnLocation).GetSafeNormal();
	LaunchDirection.Z += 0.2f;
	LaunchDirection.Normalize();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = PC;
	SpawnParams.Instigator = PC;
	
	AGrenadeActor* SpawnedGrenade = GetWorld()->SpawnActor<AGrenadeActor>(GrenadeActor, SpawnLocation, LaunchDirection.Rotation(), SpawnParams);

	if (SpawnedGrenade && SpawnedGrenade->GetProjectileMovementComponent())
	{
		float LaunchSpeed = 1500.f; 
		SpawnedGrenade->GetProjectileMovementComponent()->Velocity = LaunchDirection * LaunchSpeed;
		SpawnedGrenade->SetBaseDamage(BaseDamage);
	}
}

float UPTWGA_Grenade::GetGrenadeBaseDamage(APTWPlayerCharacter* PC)
{
	UPTWActiveItemInstance* ActiveInst = InventoryComponent->GetCurrentActiveItemSlot();
	if (!ActiveInst) return 0;
	
	return ActiveInst->GetActiveItemBaseDamage();
}
