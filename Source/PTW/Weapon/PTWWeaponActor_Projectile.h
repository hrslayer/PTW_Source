// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWWeaponData.h"
#include "../Weapon/PTWWeaponActor.h"
#include "PTWWeaponActor_Projectile.generated.h"

class APTWProjectile;
class UProjectileMovementComponent;

UCLASS()
class PTW_API APTWWeaponActor_Projectile : public APTWWeaponActor
{
	GENERATED_BODY()

public:
	APTWWeaponActor_Projectile();
	FORCEINLINE TSubclassOf<APTWProjectile> GetProjectile() const { return ProjectileClass; }

protected:
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<APTWProjectile> ProjectileClass;
};
