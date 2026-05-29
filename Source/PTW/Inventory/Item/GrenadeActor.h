// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrenadeActor.generated.h"

class UGameplayEffect;
class UProjectileMovementComponent;
class UBoxComponent;

UCLASS()
class PTW_API AGrenadeActor : public AActor
{
	GENERATED_BODY()

public:
	AGrenadeActor();
	
	FORCEINLINE UProjectileMovementComponent* GetProjectileMovementComponent() const { return ProjectileMovementComponent; }
	
	FORCEINLINE void SetBaseDamage(float Damage) {BaseDamage = Damage;}
protected:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	void Explode();
	
	void ApplyExplosionDamage(AActor* TargetActor);
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UBoxComponent> BoxComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExplodeRadius = 500.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> DamageGEClass;
};
