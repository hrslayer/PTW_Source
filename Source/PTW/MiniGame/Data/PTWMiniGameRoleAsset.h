// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreFramework/PTWPlayerData.h"
#include "Engine/DataAsset.h"
#include "PTWMiniGameRoleAsset.generated.h"

UENUM(BlueprintType)
enum class EPTWRoleCountMode : uint8
{
	Fixed       UMETA(DisplayName = "고정 인원"),
	Remainder   UMETA(DisplayName = "나머지 전부"),
	Ratio       UMETA(DisplayName = "비율"),
};

USTRUCT(BlueprintType)
struct FPTWRoleRatio
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTag Role;

	UPROPERTY(EditAnywhere)
	EPTWRoleCountMode CountMode = EPTWRoleCountMode::Fixed;

	UPROPERTY(EditAnywhere)
	int32 Count = 1;
};

UCLASS()
class PTW_API UPTWMiniGameRoleAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray<FPTWRoleData> RoleData;

	UPROPERTY(EditAnywhere)
	TArray<FPTWRoleRatio> RoleRatios;
};
