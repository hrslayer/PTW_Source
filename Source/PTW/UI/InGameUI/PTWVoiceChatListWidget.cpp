#include "PTWVoiceChatListWidget.h"
#include "PTWVoiceChatWidget.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "GameFramework/PlayerState.h"
#include "System/PTWVoiceChatSubsystem.h"


void UPTWVoiceChatListWidget::InitializeWidget()
{
	if (!GetOwningPlayerState() || !GetOwningPlayerState()->GetUniqueId().IsValid() || 
		GetOwningPlayerState()->GetPlayerName().IsEmpty() || GetOwningPlayerState()->GetPlayerName() == TEXT("Player"))
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::InitializeWidget, 0.25f, false);
		return;
	}
	
	UPTWVoiceChatSubsystem* VoiceChatSubsystem = UPTWVoiceChatSubsystem::Get(this);
	if (!IsValid(VoiceChatSubsystem)) return;
	
	VoiceChatSubsystem->OnVoiceChatConnected.AddUniqueDynamic(this, &ThisClass::HandlePlayerConnected);
	VoiceChatSubsystem->OnVoiceChatDisconnected.AddUniqueDynamic(this, &ThisClass::HandlePlayerDisconnected);
	
	VoiceChatSubsystem->OnPlayerVoiceInfoUpdated.AddUniqueDynamic(this, &ThisClass::HandlePlayerVoiceInfoUpdated);
	
	TMap<FString, FPTWPlayerVoiceInfo>& PlayerVoiceInfoList = VoiceChatSubsystem->PlayerVoiceInfoList;
	for (auto PlayerVoiceInfoEnty : PlayerVoiceInfoList)
	{
		const FString& UniqueId =  PlayerVoiceInfoEnty.Key;
		HandlePlayerConnected(UniqueId);
	}
}

void UPTWVoiceChatListWidget::HandlePlayerVoiceInfoUpdated(const FString& UniqueId)
{
	if (UniqueId.IsEmpty() || !IsValid(VoiceChatList) || !PlayerVoiceChats.Contains(UniqueId)) return;
	
	UPTWVoiceChatSubsystem* VoiceChatSubsystem = UPTWVoiceChatSubsystem::Get(this);
	if (!IsValid(VoiceChatSubsystem)) return;
	
	FPTWPlayerVoiceInfo* PlayerVoiceInfo = VoiceChatSubsystem->PlayerVoiceInfoList.Find(UniqueId);
	if (!PlayerVoiceInfo) return;
	
	EPTWVoiceState& VoiceState = PlayerVoiceInfo->VoiceState;
	
	bool bVisibility = VoiceState == EPTWVoiceState::Talking || VoiceState == EPTWVoiceState::Requesting;
	
	
	UPTWVoiceChatWidget* TargetWidget = PlayerVoiceChats[UniqueId];
	if (!IsValid(TargetWidget)) return;
	
	if (UPanelWidget* ParentWrapper = TargetWidget->GetParent())
	{
		ESlateVisibility NewVisibility = bVisibility ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
		ParentWrapper->SetVisibility(NewVisibility);
	}
	
	if (VoiceState == EPTWVoiceState::Talking)
	{
		TargetWidget->SetTalkingVoiceIcon();
	}
	else if (VoiceState == EPTWVoiceState::Requesting)
	{
		TargetWidget->SetEnabledVoiceIcon();
	}
}

void UPTWVoiceChatListWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitializeWidget();
	if (UPTWVoiceChatSubsystem* VoiceChatSubsystem = UPTWVoiceChatSubsystem::Get(this))
	{
		VoiceChatSubsystem->OnVoiceChatWidgetReady.Broadcast();
	}
}

void UPTWVoiceChatListWidget::NativeDestruct()
{
	if (UPTWVoiceChatSubsystem* VoiceChatSubsystem = UPTWVoiceChatSubsystem::Get(this))
	{
		VoiceChatSubsystem->OnVoiceChatConnected.RemoveDynamic(this, &ThisClass::HandlePlayerConnected);
		VoiceChatSubsystem->OnVoiceChatDisconnected.RemoveDynamic(this, &ThisClass::HandlePlayerDisconnected);
		
		VoiceChatSubsystem->OnPlayerVoiceInfoUpdated.RemoveDynamic(this, &ThisClass::HandlePlayerVoiceInfoUpdated);
	}
	
	Super::NativeDestruct();
}

void UPTWVoiceChatListWidget::HandlePlayerConnected(const FString& UniqueId)
{
	UPTWVoiceChatSubsystem* VoiceChatSubsystem = UPTWVoiceChatSubsystem::Get(this);
	if (!IsValid(VoiceChatSubsystem)) return;
	
	TMap<FString, FPTWPlayerVoiceInfo>& PlayerVoiceInfoList = VoiceChatSubsystem->PlayerVoiceInfoList;
	FPTWPlayerVoiceInfo* PlayerVoiceInfo = PlayerVoiceInfoList.Find(UniqueId);
	if (!PlayerVoiceInfo) return;
	
	UPTWVoiceChatWidget* TargetWidget = nullptr;
	if (!PlayerVoiceChats.Contains(UniqueId))
	{
		USizeBox* SizeBox = NewObject<USizeBox>(this, USizeBox::StaticClass());
		SizeBox->SetHeightOverride(32.0f);
	
		TargetWidget = CreateWidget<UPTWVoiceChatWidget>(this, VoiceChatWidgetClass);
		TargetWidget->InitializeWidget(PlayerVoiceInfo->PlayerName);
		SizeBox->AddChild(TargetWidget);
	
		PlayerVoiceChats.Add(UniqueId, TargetWidget);
		VoiceChatList->AddChildToVerticalBox(SizeBox);
	}
	else
	{
		TargetWidget = PlayerVoiceChats[UniqueId];
	}
	
	const FString& LocalUniqueId = GetOwningPlayerState()->GetUniqueId().ToString();
	if (LocalUniqueId == UniqueId)
	{
		TargetWidget->SetEnabledVoiceIcon();
	}
	else
	{
		TargetWidget->SetTalkingVoiceIcon();
	}
	
	if (UPanelWidget* ParentWrapper = TargetWidget->GetParent())
	{
		ParentWrapper->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPTWVoiceChatListWidget::HandlePlayerDisconnected(const FString& UniqueId)
{
	if (PlayerVoiceChats.Contains(UniqueId))
	{
		if (UPanelWidget* ParentWrapper = PlayerVoiceChats[UniqueId]->GetParent())
		{
			ParentWrapper->RemoveFromParent();
		}
		PlayerVoiceChats.Remove(UniqueId);
	}
}

