#include "UI/OptionsWidget/PTWVoiceVolume.h"
#include "Components/EditableText.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "System/PTWVoiceChatSubsystem.h"

void UPTWVoiceVolume::InitWidget(const FString& UniqueId, const FPTWPlayerVoiceInfo& PlayerVoiceInfo)
{
	if (UniqueId.IsEmpty() || PlayerVoiceInfo.PlayerName.IsEmpty()) return;
	
	PlayerId = UniqueId;
	
	if (Text_PlayerName)
	{
		Text_PlayerName->SetText(FText::FromString(PlayerVoiceInfo.PlayerName));
	}
	
	if (UPTWVoiceChatSubsystem* VoiceSubsystem = UPTWVoiceChatSubsystem::Get(this))
	{
		float CurrentVol = VoiceSubsystem->GetPlayerVoiceVolume(PlayerId);
		
		if (Slider_VoiceVolume)
		{
			Slider_VoiceVolume->SetValue(CurrentVol); 
		}
		
		if (ET_VoiceVolume) 
		{
			ET_VoiceVolume->SetText(FormatFloatToText(CurrentVol * 100.f));
		}
	}
}

void UPTWVoiceVolume::NativeConstruct()
{
	Super::NativeConstruct();
	
	Super::NativeConstruct();

	if (Slider_VoiceVolume)
	{
		Slider_VoiceVolume->OnValueChanged.AddDynamic(this, &UPTWVoiceVolume::OnVolumeChanged);
	}
	if (ET_VoiceVolume)
	{
		ET_VoiceVolume->OnTextCommitted.AddDynamic(this, &UPTWVoiceVolume::OnVolumeTextCommitted);
	}
}

void UPTWVoiceVolume::OnVolumeChanged(float Value)
{
	if (ET_VoiceVolume)
	{
		ET_VoiceVolume->SetText(FormatFloatToText(Value));
	}
	
	if (UPTWVoiceChatSubsystem* VoiceSubsystem = UPTWVoiceChatSubsystem::Get(this))
	{
		VoiceSubsystem->SetPlayerVoiceVolume(PlayerId, Value);
	}
}

void UPTWVoiceVolume::OnVolumeTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	FString StringValue = Text.ToString();
	if (StringValue.IsEmpty())
	{
		if (Slider_VoiceVolume) ET_VoiceVolume->SetText(FormatFloatToText(Slider_VoiceVolume->GetValue() * 100.f));
		return;
	}

	float NewValue = FCString::Atof(*StringValue);

	// 입력값이 0인데 "0"이 아닌 문자열인 경우 예외처리
	if (NewValue == 0.0f && !StringValue.Equals(TEXT("0")) && !StringValue.Equals(TEXT("0.0")))
	{
		if (Slider_VoiceVolume) ET_VoiceVolume->SetText(FormatFloatToText(Slider_VoiceVolume->GetValue() * 100.f));
		return;
	}

	NewValue = FMath::Clamp(NewValue, 0.0f, 200.0f);

	float SliderValue = NewValue / 100.f;
	if (Slider_VoiceVolume) Slider_VoiceVolume->SetValue(SliderValue);
	if (ET_VoiceVolume) ET_VoiceVolume->SetText(FormatFloatToText(NewValue));

	if (UPTWVoiceChatSubsystem* VoiceSubsystem = UPTWVoiceChatSubsystem::Get(this))
	{
		VoiceSubsystem->SetPlayerVoiceVolume(PlayerId, SliderValue);
	}
}

FText UPTWVoiceVolume::FormatFloatToText(float Value) const
{
	FNumberFormattingOptions NumberOptions = FNumberFormattingOptions::DefaultNoGrouping();
	NumberOptions.SetMaximumFractionalDigits(0);
	NumberOptions.SetMinimumFractionalDigits(0);

	return FText::AsNumber(Value, &NumberOptions);
}
