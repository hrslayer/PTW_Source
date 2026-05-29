// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PTWWeaponData.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EHitType : uint8
{
	HitScan,
	Projectile
};


UCLASS()
class PTW_API UPTWWeaponData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float BaseDamage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 MaxAmmo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EHitType HitType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float FireRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon") // 탄퍼짐 강도
	float SpreadIncrement = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon") // 탄퍼짐 회복 속도
	float SpreadRecoveryRate = 2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon") // 최대 탄퍼짐
	float MaxSpread = 5.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon") // 최소 탄퍼짐
	float MinSpread = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TMap<FGameplayTag, UAnimMontage*> AnimMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TMap<FGameplayTag, UAnimMontage*> WeaponAnimMap;
};
