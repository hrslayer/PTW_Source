// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/PTWMiniGameMode.h"
#include "PTWRocketGameMode.generated.h"

class APTWDropWarningLamp;
class APTWCollapseManager;
/**
 * 
 */
UCLASS()
class PTW_API APTWRocketGameMode : public APTWMiniGameMode
{
	GENERATED_BODY()
	
public:
	APTWRocketGameMode();
	
	virtual void StartRound() override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void HandlePlayerDeath(AActor* DeadActor, AActor* KillActor) override;
	
	void SetMiniGameRule();
	
	void GiveRocketWeapon();
	
	void SetSpectatorPawn(AController* Controller);
	
	void StopDropWarning();
	
	void KillPlayerPossess(AActor* KillActor);
	
protected:
	virtual void BeginPlay() override;
	
	void StartCollapseTimer();
	
	void ExecuteCollapseSequence();

	void StartDropWarning();
	
	int32 GetCollapseTiles();
	
private:
	void SetAlivePlayerStates();
	
	int32 GetRandomIndex(int32 TileSize);
	
	void StopCollapseSequence();

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UPTWItemDefinition> RocketDefinition;
	
	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<APTWPlayerState>> DeadPlayerStates;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<APTWCollapseManager> CollaspeManager;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<APTWDropWarningLamp> WarningLamp;

	TArray<APTWPlayerState*> OutAlive;
private:
	FTimerHandle CollapseTimer;
	
	int32 AlivePlayerCounts;
};
