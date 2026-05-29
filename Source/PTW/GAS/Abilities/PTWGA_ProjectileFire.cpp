// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_ProjectileFire.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/Instance/PTWWeaponInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/PTWProjectile.h"
#include "Weapon/PTWWeaponActor_Projectile.h"

void UPTWGA_ProjectileFire::PerformFireAction(const FPTWFireConext Context)
{
	if (!HasAuthority(&CurrentActivationInfo)) return;
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageGEClass, GetAbilityLevel());
	
	if (!Context.PC || !Context.WeaponInst) return;
	
	FVector CameraLocation;
	FRotator CameraRotation;
	Context.PC->GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);
	
	FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * 10000.0f);
	FHitResult ScreenHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Context.PC);

	FVector TargetLocation = TraceEnd;
	if (GetWorld()->LineTraceSingleByChannel(ScreenHit, CameraLocation, TraceEnd, ECC_Visibility, Params))
	{
		TargetLocation = ScreenHit.ImpactPoint;
	}


	FVector MuzzleLocation = Context.WeaponInst->SpawnedWeapon3P->GetMuzzleComponent()->GetComponentLocation();
	FVector MuzzleForward = Context.WeaponInst->SpawnedWeapon3P->GetMuzzleComponent()->GetRightVector();
	
	FVector SpawnLocation = MuzzleLocation + (MuzzleForward * 50.0f);
	
	FRotator AdjustedRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, TargetLocation);
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = Context.PC;
	SpawnParams.Owner = Context.PC;
	
	UPTWInventoryComponent* InventoryComponent = Context.PC->GetInventoryComponent();
	if (!InventoryComponent) return;
	
	UPTWWeaponInstance* WeaponInstance = InventoryComponent->GetCurrentWeaponInst<UPTWWeaponInstance>();
	if (!WeaponInstance) return;
	
	APTWWeaponActor_Projectile* ProjectileWeapon = Cast<APTWWeaponActor_Projectile>(WeaponInstance->SpawnedWeapon3P);
	if (!ProjectileWeapon) return;
	
	APTWProjectile* Bullet = GetWorld()->SpawnActor<APTWProjectile>(ProjectileWeapon->GetProjectile(), SpawnLocation, AdjustedRotation, SpawnParams);
	
	if (Bullet)
	{
		Bullet->DamageSpecHandle = SpecHandle;
	}
}
