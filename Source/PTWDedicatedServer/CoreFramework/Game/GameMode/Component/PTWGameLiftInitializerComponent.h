#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameLiftServerSDK.h"
#include "PTWGameLiftInitializerComponent.generated.h"


struct FComponentRequestHandle;

/*
 * 게임리프트 로직을 초기화하는 서버 전용 컴포넌트 입니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PTWDEDICATEDSERVER_API UPTWGameLiftInitializerComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void HandleOwnerExtension(AActor* Receiver, FName EventName);
	
	UFUNCTION()
	void InitializeGameLift();

public:
	TSharedPtr<FProcessParameters> ProcessParameters;
	
protected:
	TSharedPtr<FComponentRequestHandle> OwnerExtensionHandle;
	
};
