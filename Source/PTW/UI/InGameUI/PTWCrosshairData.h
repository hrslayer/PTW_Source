// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PTWCrosshairData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FCrosshairData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	// 옵션 창에 표시될 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText CrosshairName; 

	// WBP의 클래스 타입을 담는 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<UUserWidget> CrosshairWidgetClass;
};
