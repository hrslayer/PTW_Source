#pragma once
#include "CoreMinimal.h"
#include "CoreFramework/Game/GameMode/PTWGameMode.h"
#include "PTWStagingGameMode.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAutoBackfillTimeoutRemainTime, const int32, RemainTime);

/*
 * 로비로 진입하기 전 플레이어들을 모으고 시작 가능여부를 확인하는 게임모드 입니다.
 */
UCLASS()
class PTW_API APTWStagingGameMode : public APTWGameMode
{
	GENERATED_BODY()

public:
	APTWStagingGameMode();
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void UpdateTimer() override;
public:

private:
	UPROPERTY(EditDefaultsOnly)
	float WaitingTime = 60.f;
	
public:
	FOnAutoBackfillTimeoutRemainTime OnAutoBackfillTimeoutRemainTime;
};
