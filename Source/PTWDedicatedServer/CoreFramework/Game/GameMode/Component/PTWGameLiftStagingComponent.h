#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWGameLiftStagingComponent.generated.h"


struct FComponentRequestHandle;

/*
 * 서버 백필과 요구 플레이어 수 확인을 관리하는 컴포넌트입니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PTWDEDICATEDSERVER_API UPTWGameLiftStagingComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void HandleOwnerExtension(AActor* Receiver, FName EventName);
	
	UFUNCTION()
	void HandleAutoBackfillTimeout(int32 RemainTime);
	
	UFUNCTION()
	void HandleEndTimer();
	
	UFUNCTION()
	void HandlePostLogin(APlayerController* NewPlayerController);
	UFUNCTION()
	void TerminateGameSession();
	
	
protected:
	TSharedPtr<FComponentRequestHandle> OwnerExtensionHandle;
	
	FTimerHandle AbandonedSCheckTimerHandle;
	FTimerHandle TerminationTimerHandle;
};
