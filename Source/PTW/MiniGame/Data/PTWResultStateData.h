// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


UENUM(BlueprintType)
enum class EPTWResultStatName : uint8
{
	Bomb_PassCount      UMETA(DisplayName = "폭탄 넘긴 횟수"),
	Bomb_ReceiveCount   UMETA(DisplayName = "폭탄 받은 횟수"),
	
	AbilityBattle_Damage UMETA(DisplayName = "적에게 입힌 피해량"),
	
	Rocket_Kill UMETA(DisplayName = "폭탄으로 적을 낙사 시킨 횟수"),

	GhostChase_Damage       UMETA(DisplayName = "타겟에게 입힌 피해량"),
	GhostChase_ChasedCount  UMETA(DisplayName = "추적한 타겟 수"),
	GhostChase_ChaserCount  UMETA(DisplayName = "나를 추적한 사람 수")
};

UENUM(BlueprintType)  
enum class EPTWResultStatTopName : uint8
{
	Bomb_PassCount      UMETA(DisplayName = "가장 많은 폭탄을 넘긴 플레이어"),
	Bomb_ReceiveCount   UMETA(DisplayName = "가장 많은 폭탄을 받은 플레이어"),
	
	AbilityBattle_Damage UMETA(DisplayName = "가장 많은 피해를 입힌 플레이어"),
	
	Rocket_Kill UMETA(DisplayName = "가장 많이 폭탄으로 적을 낙사 시킨 횟수"),

	GhostChase_Damage       UMETA(DisplayName = "최고의 딜러"),
	GhostChase_ChasedCount  UMETA(DisplayName = "가장 많은 사람을 추적한 플레이어"),
	GhostChase_ChaserCount  UMETA(DisplayName = "가장 많은 사람에게 추적당한 플레이어")
};
