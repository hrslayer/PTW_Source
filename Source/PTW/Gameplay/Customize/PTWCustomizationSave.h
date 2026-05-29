// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Gameplay/Customize/CustomizeData.h"
#include "PTWCustomizationSave.generated.h"

UCLASS()
class PTW_API UPTWCustomizationSave : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, Category = "Customization")
	EMeshGender EquippedGender;

	UPROPERTY(VisibleAnywhere, Category = "Customization")
	FName EquippedUpperAddonID;
	UPROPERTY(VisibleAnywhere, Category = "Customization")
	FName EquippedBackAddonID;
	UPROPERTY(VisibleAnywhere, Category = "Customization")
	FName EquippedBodyID;
	UPROPERTY(VisibleAnywhere, Category = "Customization")
	FName EquippedEyewearID;
	UPROPERTY(VisibleAnywhere, Category = "Customization")
	FName EquippedGlovesID;
	UPROPERTY(VisibleAnywhere, Category = "Customization")
	FName EquippedHairID;
	UPROPERTY(VisibleAnywhere, Category = "Customization")
	FName EquippedHatID;
	UPROPERTY(VisibleAnywhere, Category = "Customization")
	FName EquippedHeadID;
	UPROPERTY(VisibleAnywhere, Category = "Customization")
	FName EquippedLowerID;
	UPROPERTY(VisibleAnywhere, Category = "Customization")
	FName EquippedShoesID;
};
