// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/PTWProjectile.h"
#include "PTWKnockBackProjectile.generated.h"

UCLASS()
class PTW_API APTWKnockBackProjectile : public APTWProjectile
{
	GENERATED_BODY()

public:
	APTWKnockBackProjectile();

protected:
	virtual void BeginPlay() override;
	
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	
	void ApplyKnockBack(TArray<FOverlapResult>& OverlapResults);
	
protected:
	UPROPERTY(EditAnywhere, Category = "KnockBack Value")
	float MaxLaunchStrength;
	
	UPROPERTY(EditAnywhere, Category = "KnockBack Value")
	float MaxUpwardForce;
};
