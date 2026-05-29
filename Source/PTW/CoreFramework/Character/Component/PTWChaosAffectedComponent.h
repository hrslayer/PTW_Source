// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWChaosAffectedComponent.generated.h"


class APTWGameState;
class UCharacterMovementComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PTW_API UPTWChaosAffectedComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPTWChaosAffectedComponent();

	void ApplyCurrentChaosEvent();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetIceFloorEvent(bool bEnable);

	UFUNCTION(Client, Reliable)
	void Client_SetInputState(const bool bFrozen,const bool bInvert);
	
	// 마우스 회전 관련 
	bool bLookActionFrozen = false;
	bool bInputInvert = false;
protected:
	virtual void BeginPlay() override;

private:
	UCharacterMovementComponent* GetOwnerMovement();
	
	void CacheDefaultMovementValues();
	
	// 빙판길 이벤트 관련 데이터
	float DefaultGroundFriction;
	float DefaultBrakingDeceleration;
	
};
