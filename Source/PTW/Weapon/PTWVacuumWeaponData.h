// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/PTWWeaponData.h"
#include "PTWVacuumWeaponData.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWVacuumWeaponData : public UPTWWeaponData
{
	GENERATED_BODY()
	
public:
	// 흡입 거리
	UPROPERTY(EditAnywhere, Category = "Vacuum|Suction")
	float VacuumRadius = 800.0f;     
	// 부채꼴 반각 (총 90도 범위)
	UPROPERTY(EditAnywhere, Category = "Vacuum|Suction")
	float VacuumAngle = 45.0f;       
	// 흡입 강도
	UPROPERTY(EditAnywhere, Category = "Vacuum|Suction")
	float SuctionStrength = 5000.0f; 
	// 발사(틱)당 스태미나(탄약) 소모량
	UPROPERTY(EditAnywhere, Category = "Vacuum|Stamina")
	int32 DrainAmountPerTick = 2;    
	// 쉬는 동안(틱) 회복량
	UPROPERTY(EditAnywhere, Category = "Vacuum|Stamina")
	int32 RegenAmountPerTick = 1;    
	// 회복 타이머 주기
	UPROPERTY(EditAnywhere, Category = "Vacuum|Stamina")
	float RegenInterval = 0.1f;     
	// 과열 시 락다운 시간
	UPROPERTY(EditAnywhere, Category = "Vacuum|Overheat")
	float OverheatDuration = 5.0f;
};
