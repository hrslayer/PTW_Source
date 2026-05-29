#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWSpectatorHUD.generated.h"


class UBorder;
class UTextBlock;

/**
 * 관전 시, 표시될 UI를 표시합니다.
 */
UCLASS()
class PTW_API UPTWSpectatorHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void SetSpectateTargetASC(const APlayerState* TargetPlayerState);
	void HandleNativeVisibilityChanged(ESlateVisibility InVisibility);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> SpectateTargetBorder;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> SpectateTargetText;
	
private:
	
};
