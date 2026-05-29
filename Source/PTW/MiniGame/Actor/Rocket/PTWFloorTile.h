// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWFloorTile.generated.h"

class UTimelineComponent;

UCLASS()
class PTW_API APTWFloorTile : public AActor
{
	GENERATED_BODY()

public:
	APTWFloorTile();
	
	void StartCollapse();                                                                                                   

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void HandleCollapseUpdate(float Value);
	
	UFUNCTION()
	void HandleCollapseFinished();
	
	UFUNCTION()
	void HandleWarningFlashUpdate(float Value);
	
	UFUNCTION()
	void HandleWarningFinished();
	
	void PlayWarningEffect();
	
	void StartFalling();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_StartCollapse();
	
	void CallWarningLampStop();
	
protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USceneComponent> SceneComponent;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> SM;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UTimelineComponent> FallTimeline;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UTimelineComponent> WarningTimeline;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCurveFloat> FallCurve;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCurveFloat> WarningCurve;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMat;
	
	FVector StartLocation;
	
private:
	FTimerHandle WarningTimerHandle;
};
