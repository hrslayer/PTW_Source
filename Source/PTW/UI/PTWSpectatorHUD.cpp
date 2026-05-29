#include "PTWSpectatorHUD.h"
#include "PTWInGameHUD.h"
#include "PTWUISubsystem.h"
#include "Components/TextBlock.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/PTWSpectatorPawn.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "GameFramework/PlayerState.h"


void UPTWSpectatorHUD::SetSpectateTargetASC(const APlayerState* TargetPlayerState)
{
	// SpectateTargetBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (!IsValid(TargetPlayerState)) return;
	
	UPTWGameInstance* GI = GetGameInstance<UPTWGameInstance>();
	if (!IsValid(GI)) return;
	
	const FString TargetUniqueId = TargetPlayerState->GetUniqueId().IsValid() ? TargetPlayerState->GetUniqueId().ToString() : TEXT("");
	if (TargetUniqueId.IsEmpty()) return;
	
	const auto& PlayerNames = GI->SessionPlayerNames;
	if (!PlayerNames.Contains(TargetUniqueId)) return;
	
	SpectateTargetText->SetText(FText::FromString(PlayerNames[TargetUniqueId]));
	
	const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetPlayerState);
	if (!ASI) return;
	
	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) return;
	
	const ULocalPlayer* LP = GetOwningLocalPlayer();
	if (!IsValid(LP)) return;
		
	UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>();
	if (!IsValid(UISubsystem)) return;

	UISubsystem->TryInitializeHUDASC(ASC);
}

void UPTWSpectatorHUD::HandleNativeVisibilityChanged(ESlateVisibility InVisibility)
{
	APTWPlayerController* PC = GetOwningPlayer<APTWPlayerController>();
	if (!IsValid(PC)) return;

	switch (InVisibility)
	{
	case ESlateVisibility::Visible:
	case ESlateVisibility::HitTestInvisible:
	case ESlateVisibility::SelfHitTestInvisible:
		// SpectateTargetBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
		break;
	
	case ESlateVisibility::Hidden:
	case ESlateVisibility::Collapsed:
		// SpectateTargetBorder->SetVisibility(ESlateVisibility::Collapsed);
	default:
		break;
	}
}

void UPTWSpectatorHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
	// OnNativeVisibilityChanged.AddUObject(this, &ThisClass::HandleNativeVisibilityChanged);
	if (APTWPlayerController* PC = GetOwningPlayer<APTWPlayerController>())
	{
		if ( APTWSpectatorPawn* SP = PC->GetSpectatorPawn() ? Cast<APTWSpectatorPawn>(PC->GetSpectatorPawn()) : nullptr)
		{
			SP->OnSpectateTargetChanged.AddUniqueDynamic(this, &ThisClass::SetSpectateTargetASC);
		}
	}
}

void UPTWSpectatorHUD::NativeDestruct()
{
	// OnNativeVisibilityChanged.RemoveAll(this);
	if (APTWPlayerController* PC = GetOwningPlayer<APTWPlayerController>())
	{
		if ( APTWSpectatorPawn* SP = PC->GetSpectatorPawn() ? Cast<APTWSpectatorPawn>(PC->GetSpectatorPawn()) : nullptr)
		{
			SP->OnSpectateTargetChanged.RemoveAll(this);
		}
	}
	
	Super::NativeDestruct();
}
