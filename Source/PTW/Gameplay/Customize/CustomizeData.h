// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CustomizeData.generated.h"

UENUM(BlueprintType)
enum class EMeshGender : uint8
{
	Male,
	Female
};

UENUM(BlueprintType)
enum class EMeshType : uint8
{
	None,
	UpperAddon		UMETA(DisplayName = "UpperAddon"),
	BackAddon		UMETA(DisplayName = "BackAddon"),
	Body		UMETA(DisplayName = "Body"),
	Eyewear		UMETA(DisplayName = "Eyewear"),
	Gloves		UMETA(DisplayName = "Gloves"),
	Hair		UMETA(DisplayName = "Hair"),
	Hat		UMETA(DisplayName = "Hat"),
	Head		UMETA(DisplayName = "Head"),
	Lower		UMETA(DisplayName = "Lower"),
	Shoes		UMETA(DisplayName = "Shoes")
};

USTRUCT(BlueprintType)
struct FCustomizeDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMeshGender Gender;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMeshType MeshType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USkeletalMesh> DisplayMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayText;
};
