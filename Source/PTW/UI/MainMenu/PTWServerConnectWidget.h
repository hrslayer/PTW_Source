#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWServerConnectWidget.generated.h"


class UWidgetSwitcher;
class UCircularThrobber;
class UTextBlock;
class UHorizontalBox;
class UButton;

#define TIMER_START_INDEX 0
#define TIMER_ACCEPT_INDEX 1

#define CANCEL_BUTTON_INDEX 0
#define REJECT_BUTTON_INDEX 1

/**
 * 서버에 접속할 때, 화면에 팝업되는 위젯입니다.
 */
UCLASS()
class PTW_API UPTWServerConnectWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void HandleStartTimer();
	void HandleAcceptTimer();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void HandleMatchmakingFlowChanged();
	void UpdateAcceptanceProgress(int32 TotalPlayers, int32 AcceptedPlayers);
	
	void HandleFlowNone();
	void HandleFlowQueued();
	void HandleFlowSearching();
	void HandleFlowMatchFound();
	void HandleFlowRequiresAcceptance();
	void HandleFlowPlacing();
	void HandleFlowCompleted();
	void HandleFlowTimedOut();
	void HandleFlowCancelled();
	void HandleFlowFailed();
	
	UFUNCTION()
	void OnClickedAccept();
	UFUNCTION()
	void OnClickedCancel();
	UFUNCTION()
	void OnClickedReject();
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWidgetSwitcher> TimerSwitcher;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCircularThrobber> StartMatchThrobber;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> StartTimerText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCircularThrobber> AcceptMatchThrobber;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> AcceptTimerText;
	
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> MessageText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> AcceptanceHorizontal;
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> AcceptedTexture;
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> PendingTexture;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> AcceptButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWidgetSwitcher> NoButtonSwitcher;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CancelButton;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> RejectButton;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Timer")
	int32 StartTimer = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Timer")
	int32 AcceptTimer = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Timer")
	bool bShowTimerTick = true;
	
protected:
	FTimerHandle StartTimerHandle;
	FTimerHandle AcceptanceTimeoutHandle;
};
