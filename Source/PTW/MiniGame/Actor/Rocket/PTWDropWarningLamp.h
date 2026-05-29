// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWDropWarningLamp.generated.h"

class USpotLightComponent;

UCLASS()
class PTW_API APTWDropWarningLamp : public AActor
{
	GENERATED_BODY()

public:
	APTWDropWarningLamp();
	
	void StartDropWarningLamp();
	
	void StopDropWarningLamp();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_StartWarningLight();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_StopWarningLight();
	
	virtual void Tick(float DeltaSeconds);

protected:
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USceneComponent> SceneComponent;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> LampMesh;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<USpotLightComponent> SpotLight;
	
	UPROPERTY(EditAnywhere)
	float RotationSpeed = 150.0f;
	
	bool bIsWarning = false;
};
