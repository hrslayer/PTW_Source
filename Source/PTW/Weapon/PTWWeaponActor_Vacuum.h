// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/PTWWeaponActor.h"
#include "Components/BoxComponent.h"
#include "PTWWeaponActor_Vacuum.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API APTWWeaponActor_Vacuum : public APTWWeaponActor
{
	GENERATED_BODY()
	
public:
	APTWWeaponActor_Vacuum();

	// 상위 무기 클래스의 재장전 이벤트를 가로채서 아무것도 하지 못하게 원천 차단
	virtual void HandleReloadEvent(EReloadEventAction ActionType) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSuctionZoneOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vacuum")
	TObjectPtr<UBoxComponent> SuctionZone;
};
