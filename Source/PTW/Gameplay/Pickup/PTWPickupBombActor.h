// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/PTWPickupItemBase.h"
#include "PTWPickupBombActor.generated.h"

class UGameplayEffect;
/**
 * 
 */
UCLASS()
class PTW_API APTWPickupBombActor : public APTWPickupItemBase
{
	GENERATED_BODY()
	
public:
	APTWPickupBombActor();

protected:
	virtual void OnPickedUp(class APTWPlayerCharacter* Player) override;

protected:
	/* 폭발 범위 (반경) - 에디터에서 조절 가능 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|Settings")
	float ExplosionRadius = 300.0f;

	/* 폭발 데미지 - 에디터에서 조절 가능 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|Settings")
	float BombDamage = 25.0f;

	// 수평으로 밀어내는 최대 힘
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|Settings")
	float BombLaunchStrength = 1500.0f;

	// 수직으로 붕 띄우는 최대 힘
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|Settings")
	float BombUpwardForce = 500.0f;

	/* 폭발 데미지 적용용 GE */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|GAS")
	TSubclassOf<UGameplayEffect> BombDamageGEClass;

	/* 과열 태그용 GE */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|GAS")
	TSubclassOf<UGameplayEffect> BombOverheatGEClass;
};
