// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWPreviewActor.generated.h"

UCLASS()
class PTW_API APTWPreviewActor : public AActor
{
	GENERATED_BODY()
	
public:
	APTWPreviewActor();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Customization")
	TObjectPtr<USkeletalMeshComponent> BaseMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Customization")
	TObjectPtr<USkeletalMeshComponent> SK_UpperAddon;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Customization")
	TObjectPtr<USkeletalMeshComponent> SK_BackAddon;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Customization")
	TObjectPtr<USkeletalMeshComponent> SK_Eyewear;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Customization")
	TObjectPtr<USkeletalMeshComponent> SK_Gloves;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Customization")
	TObjectPtr<USkeletalMeshComponent> SK_Hair;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Customization")
	TObjectPtr<USkeletalMeshComponent> SK_Hat;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Customization")
	TObjectPtr<USkeletalMeshComponent> SK_Head;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Customization")
	TObjectPtr<USkeletalMeshComponent> SK_Lower;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Customization")
	TObjectPtr<USkeletalMeshComponent> SK_Shoes;

};
