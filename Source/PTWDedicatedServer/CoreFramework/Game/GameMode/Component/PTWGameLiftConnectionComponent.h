#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWGameLiftConnectionComponent.generated.h"


struct FComponentRequestHandle;

/*
 * 플레이어들의 접속/종료를 관리하는 서버 전용 컴포넌트 입니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PTWDEDICATEDSERVER_API UPTWGameLiftConnectionComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void HandleOwnerExtension(AActor* Receiver, FName EventName);
	
	UFUNCTION()
	void HandlePreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage);
	UFUNCTION()
	void HandlePostLogin(APlayerController* NewPlayerController);
	UFUNCTION()
	void HandlePreLogout(AController* Exiting);
	UFUNCTION()
	void HandlePostLogout(AController* Exiting);

	UFUNCTION()
	void TerminateGameSession();
	
protected:
	TSharedPtr<FComponentRequestHandle> OwnerExtensionHandle;
};
