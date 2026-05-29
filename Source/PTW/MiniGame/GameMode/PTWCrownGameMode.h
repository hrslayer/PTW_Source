#pragma once
#include "CoreMinimal.h"
#include "MiniGame/PTWMiniGameMode.h"
#include "CoreFramework/PTWPlayerState.h"
#include "MiniGame/Actor/Crown/PTWCrown.h"
#include "PTWCrownGameMode.generated.h"


/**
 * 왕관뺏기 미니게임 게임모드입니다.
 */

UCLASS()
class PTW_API APTWCrownGameMode : public APTWMiniGameMode
{
	GENERATED_BODY()
	
public:
	APTWCrownGameMode();
	
	UFUNCTION(BlueprintCallable) FORCEINLINE
	APTWPlayerState* GetCurrentKing() const { return CurrentKing; };
	
	FORCEINLINE void SetSpawnedCrown(APTWCrown* Crown) { SpawnedCrown = Crown; };
	
	UFUNCTION()
	void RemovePrevKing();
	void SetPreliminaryKing(APTWPlayerState* NewKing);
	
	UFUNCTION()
	void ConfirmPreliminaryKing();
	void RandomSelectKing();
	
	void CleanupCrownSystem();
protected:
	virtual void BeginPlay() override;
	virtual void StartPlay() override;
	virtual void WaitingToStartRound() override;
	virtual void StartRound() override;
	virtual void HandlePlayerDeath(AActor* Victim, AActor* Attacker) override;
	virtual void EndGame() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void ScoreTimer();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Variables")
	TObjectPtr<APTWPlayerState> CurrentKing;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Variables")
	TObjectPtr<APTWPlayerState> PreliminaryKing;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Variables")
	TObjectPtr<APTWCrown> SpawnedCrown;
	
	UPROPERTY(EditAnywhere, Category="GAS|GameplayEffects")
	TSubclassOf<UGameplayEffect> KingBuffGameplayEffect;
	
	UPROPERTY(EditAnywhere, Category="GAS|GameplayEffects")
	TSubclassOf<UGameplayEffect> InvincibleGameplayEffect;

	// 왕관 획득 / 왕관 탈취를 체크
	bool bIsFirstCrown = true;

public:
	FTimerHandle ScoreTimerHandle;
};
