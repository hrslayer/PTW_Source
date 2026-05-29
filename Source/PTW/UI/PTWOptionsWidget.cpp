// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PTWOptionsWidget.h"

#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableText.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/Border.h"

#include "Engine/Engine.h"
#include "Engine/DataTable.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

#include "CoreFramework/PTWGameUserSettings.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/MainMenu/PTWMainMenuPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "OptionsWidget/PTWVoiceVolume.h"
#include "System/PTWVoiceChatSubsystem.h"
#include "UI/PTWUISubsystem.h"
#include "InGameUI/PTWCrosshairData.h"

void UPTWOptionsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	FrameRateValues = { 0.f, 60.f, 120.f, 144.f, 240.f };

	SetIsFocusable(true);

	SetupLanguageData();
	PopulateLanguageList();
	PopulateResolutionList();
	PopulateQualityList();
	PopulateFrameLimitList(); 
	PopulateVoiceInputModeList();
	InitializeUIFromCurrentSettings();
	CacheInitialSettings();
	BindEvents();
	
	if (CategorySwitcher)
	{
		CategorySwitcher->SetActiveWidgetIndex(0);
	}
}

void UPTWOptionsWidget::NativeDestruct()
{
	if (bIsDirty)
	{
		RestoreInitialSettings();
	}
	
	Super::NativeDestruct();
}

void UPTWOptionsWidget::PopulateResolutionList()
{
	if (!Combo_Resolution) return;

	Combo_Resolution->ClearOptions();

	TArray<FIntPoint> CommonResolutions =
	{
		FIntPoint(1280, 720),
		FIntPoint(1600, 900),
		FIntPoint(1920, 1080),
		FIntPoint(2560, 1440),
		FIntPoint(3840, 2160)
	};

	for (const FIntPoint& Res : CommonResolutions)
	{
		FString Option = FString::Printf(TEXT("%d x %d"), Res.X, Res.Y);
		Combo_Resolution->AddOption(Option);
	}
}

void UPTWOptionsWidget::PopulateQualityList()
{
	TArray<UComboBoxString*> QualityCombos = {
		Combo_Quality, Combo_ViewDistance, Combo_Shadow, Combo_Texture,
		Combo_Effects, Combo_PostProcess, Combo_Foliage, Combo_Shading, Combo_AntiAliasing
	};

	for (UComboBoxString* Combo : QualityCombos)
	{
		if (!Combo) continue;
		Combo->ClearOptions();
		Combo->AddOption(NSLOCTEXT("Options", "Low", "Low").ToString());
		Combo->AddOption(NSLOCTEXT("Options", "Medium", "Medium").ToString());
		Combo->AddOption(NSLOCTEXT("Options", "High", "High").ToString());
		Combo->AddOption(NSLOCTEXT("Options", "Epic", "Epic").ToString());
	}
}

void UPTWOptionsWidget::PopulateLanguageList()
{
	if (!Combo_Language) return;

	Combo_Language->ClearOptions();
	for (const auto& Pair : LanguageMap)
	{
		Combo_Language->AddOption(Pair.Key);
	}
}

void UPTWOptionsWidget::PopulateVoiceInputModeList()
{
	if (!Combo_VoiceInputMode) return;
	Combo_VoiceInputMode->ClearOptions();
	
	FString PushToTalk = NSLOCTEXT("Options", "PushToTalk", "PushToTalk").ToString();
	FString OpenMic = NSLOCTEXT("Options", "OpenMic", "OpenMic").ToString();
	
	Combo_VoiceInputMode->AddOption(PushToTalk);
	Combo_VoiceInputMode->AddOption(OpenMic);
	if (UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings()))
	{
		if (Settings->bIsPushToTalk)
		{
			Combo_VoiceInputMode->SetSelectedOption(PushToTalk);
		}
		else
		{
			Combo_VoiceInputMode->SetSelectedOption(OpenMic);
		}
	}
}

void UPTWOptionsWidget::PopulateFrameLimitList()
{
	if (!Combo_FrameLimit) return;
	Combo_FrameLimit->ClearOptions();

	Combo_FrameLimit->AddOption(NSLOCTEXT("Options", "Unlimited", "Unlimited").ToString());
	Combo_FrameLimit->AddOption(TEXT("60"));
	Combo_FrameLimit->AddOption(TEXT("120"));
	Combo_FrameLimit->AddOption(TEXT("144"));
	Combo_FrameLimit->AddOption(TEXT("240"));
}

void UPTWOptionsWidget::SetupLanguageData()
{
	LanguageMap.Empty();
	LanguageMap.Add(TEXT("English"), TEXT("en"));
	LanguageMap.Add(TEXT("한국어"), TEXT("ko"));
	// LanguageMap.Add(TEXT("日本語"), TEXT("ja"));
}

void UPTWOptionsWidget::InitializeUIFromCurrentSettings()
{
	if (!GEngine) return;

	UPTWGameUserSettings* Settings =
		Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());

	if (!Settings) return;

	if (Combo_Language)
	{
		const FString* CurrentName = LanguageMap.FindKey(Settings->SelectedLanguage);
		if (CurrentName)
		{
			Combo_Language->SetSelectedOption(*CurrentName);
		}
	}

	if (CheckBox_Windowed)
	{
		CheckBox_Windowed->SetIsChecked(
			Settings->GetFullscreenMode() == EWindowMode::Windowed);
	}

	if (Combo_Resolution)
	{
		FIntPoint Res = Settings->GetScreenResolution();
		FString ResString = FString::Printf(TEXT("%d x %d"), Res.X, Res.Y);
		Combo_Resolution->SetSelectedOption(ResString);
	}

	// 세부 그래픽 설정
	if (Combo_ViewDistance) Combo_ViewDistance->SetSelectedIndex(Settings->GetViewDistanceQuality());
	if (Combo_Shadow)       Combo_Shadow->SetSelectedIndex(Settings->GetShadowQuality());
	if (Combo_Texture)      Settings->GetTextureQuality() != -1 ? Combo_Texture->SetSelectedIndex(Settings->GetTextureQuality()) : Combo_Texture->SetSelectedIndex(0);
	if (Combo_Effects)      Combo_Effects->SetSelectedIndex(Settings->GetVisualEffectQuality());
	if (Combo_PostProcess)  Combo_PostProcess->SetSelectedIndex(Settings->GetPostProcessingQuality());
	if (Combo_Foliage)      Combo_Foliage->SetSelectedIndex(Settings->GetFoliageQuality());
	if (Combo_Shading)      Combo_Shading->SetSelectedIndex(Settings->GetShadingQuality());
	if (Combo_AntiAliasing) Combo_AntiAliasing->SetSelectedIndex(Settings->GetAntiAliasingQuality());
	if (CheckBox_VSync)     CheckBox_VSync->SetIsChecked(Settings->bUseVSync);

	if (Combo_Quality)
	{
		int32 FirstVal = Settings->GetViewDistanceQuality();
		bool bIsCustom = false;

		TArray<int32> Qualities = {
			Settings->GetShadowQuality(),
			Settings->GetTextureQuality(),
			Settings->GetVisualEffectQuality(),
			Settings->GetPostProcessingQuality(),
			Settings->GetFoliageQuality(),
			Settings->GetShadingQuality(),
			Settings->GetAntiAliasingQuality()
		};

		for (int32 Val : Qualities)
		{
			if (Val != FirstVal)
			{
				bIsCustom = true;
				break;
			}
		}

		if (bIsCustom)
		{
			FString CustomText = NSLOCTEXT("Options", "Custom", "Custom").ToString();
			if (Combo_Quality->FindOptionIndex(CustomText) == INDEX_NONE)
			{
				Combo_Quality->AddOption(CustomText);
			}
			Combo_Quality->SetSelectedOption(CustomText);
		}
		else
		{
			// 모든 세부 설정이 같으면 해당 레벨 표시 (4 이상인 경우 Epic(3)으로 클램프)
			int32 SafeLevel = FMath::Clamp(FirstVal, 0, 3);
			Combo_Quality->SetSelectedIndex(SafeLevel);
		}
	}

	if (Slider_MasterVolume && ET_MasterVolume)
	{
		Slider_MasterVolume->SetValue(Settings->MasterVolume); // 0~1 그대로 주입
		ET_MasterVolume->SetText(FormatFloatToText(Settings->MasterVolume * 100.f, true)); // 텍스트만 * 100
	}

	if (Slider_BGMVolume && ET_BGMVolume)
	{
		Slider_BGMVolume->SetValue(Settings->BGMVolume);
		ET_BGMVolume->SetText(FormatFloatToText(Settings->BGMVolume * 100.f, true));
	}

	if (Slider_SFXVolume && ET_SFXVolume)
	{
		Slider_SFXVolume->SetValue(Settings->SFXVolume);
		ET_SFXVolume->SetText(FormatFloatToText(Settings->SFXVolume * 100.f, true));
	}

	if (Slider_UIVolume && ET_UIVolume)
	{
		Slider_UIVolume->SetValue(Settings->UIVolume);
		ET_UIVolume->SetText(FormatFloatToText(Settings->UIVolume * 100.f, true));
	}

	if (Slider_VoiceVolume && ET_VoiceVolume)
	{
		Slider_VoiceVolume->SetValue(Settings->VoiceVolume);
		ET_VoiceVolume->SetText(FormatFloatToText(Settings->VoiceVolume * 100.f, true));
	}
	
	if (Combo_VoiceInputMode)
	{
		Combo_VoiceInputMode->SetSelectedIndex(!Settings->bIsPushToTalk);
	}
	
	if (Slider_MouseSensitivity && ET_MouseSensitivity)
	{
		Slider_MouseSensitivity->SetValue(Settings->MouseSensitivity);
		ET_MouseSensitivity->SetText(FormatFloatToText(Settings->MouseSensitivity));
	}

	if (CheckBox_InvertY)
	{
		CheckBox_InvertY->SetIsChecked(Settings->bInvertYAxis);
	}

	if (Slider_FOV && ET_FOV)
	{
		Slider_FOV->SetValue(Settings->FieldOfView);
		ET_FOV->SetText(FormatFloatToText(Settings->FieldOfView));
	}

	if (Combo_FrameLimit)
	{
		Combo_FrameLimit->SetSelectedIndex(Settings->MaxFrameRateIndex);
	}

	if (CheckBox_MotionBlur)
	{
		CheckBox_MotionBlur->SetIsChecked(Settings->bMotionBlur);
	}

	CurrentSelectedCrosshairIndex = Settings->GetCrosshairIndex();
	BuildCrosshairListDirect();
}

void UPTWOptionsWidget::CacheInitialSettings()
{
	UPTWGameUserSettings* Settings =
		Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());

	if (!Settings) return;

	InitialSnapshot.Resolution = Settings->GetScreenResolution();
	InitialSnapshot.WindowMode = Settings->GetFullscreenMode();
	InitialSnapshot.ScalabilityLevel = Settings->GetOverallScalabilityLevel();
	InitialSnapshot.ViewDistanceQuality = Settings->GetViewDistanceQuality();
	InitialSnapshot.ShadowQuality = Settings->GetShadowQuality();
	InitialSnapshot.TextureQuality = Settings->GetTextureQuality();
	InitialSnapshot.EffectsQuality = Settings->GetVisualEffectQuality();
	InitialSnapshot.PostProcessQuality = Settings->GetPostProcessingQuality();
	InitialSnapshot.FoliageQuality = Settings->GetFoliageQuality();
	InitialSnapshot.ShadingQuality = Settings->GetShadingQuality();
	InitialSnapshot.AntiAliasingQuality = Settings->GetAntiAliasingQuality();
	InitialSnapshot.bVSync = Settings->bUseVSync;
	InitialSnapshot.FieldOfView = Settings->FieldOfView;
	InitialSnapshot.MaxFrameRateIndex = Settings->MaxFrameRateIndex;
	InitialSnapshot.bMotionBlur = Settings->bMotionBlur;

	InitialSnapshot.MasterVolume = Settings->MasterVolume;
	InitialSnapshot.BGMVolume = Settings->BGMVolume;
	InitialSnapshot.SFXVolume = Settings->SFXVolume;
	InitialSnapshot.UIVolume = Settings->UIVolume;
	InitialSnapshot.VoiceVolume = Settings->VoiceVolume;
	InitialSnapshot.MouseSensitivity = Settings->MouseSensitivity;
	InitialSnapshot.bInvertYAxis = Settings->bInvertYAxis;
	InitialSnapshot.CrosshairIndex = Settings->GetCrosshairIndex();

	InitialSnapshot.bIsPushToTalk = Settings->bIsPushToTalk;

	bIsDirty = false;
}

void UPTWOptionsWidget::UpdateDisplaySettings()
{
	UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());
	if (!Settings) return;

	// 해상도 로직
	FString Left, Right;
	if (Combo_Resolution && Combo_Resolution->GetSelectedOption().Split(TEXT(" x "), &Left, &Right))
	{
		Settings->SetScreenResolution(FIntPoint(FCString::Atoi(*Left), FCString::Atoi(*Right)));
	}

	if (CheckBox_Windowed) Settings->SetFullscreenMode(CheckBox_Windowed->IsChecked() ? EWindowMode::Windowed : EWindowMode::Fullscreen);
	if (CheckBox_VSync) Settings->SetVSyncEnabled(CheckBox_VSync->IsChecked());

	Settings->ApplySettings(false);
	bIsDirty = true;
}

void UPTWOptionsWidget::UpdateAudioSettings()
{
	UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());
	if (!Settings) return;

	UWorld* World = GetWorld();
	if (!World || !MasterSoundMix) return;

	// 현재 UI 값 가져오기
	const float Master = Slider_MasterVolume ? Slider_MasterVolume->GetValue() : 1.f;
	const float BGM = Slider_BGMVolume ? Slider_BGMVolume->GetValue() : 1.f;
	const float SFX = Slider_SFXVolume ? Slider_SFXVolume->GetValue() : 1.f;
	const float UI = Slider_UIVolume ? Slider_UIVolume->GetValue() : 1.f;
	const float Voice = Slider_VoiceVolume ? Slider_VoiceVolume->GetValue() : 1.f;

	// Settings 저장
	Settings->MasterVolume = Master;
	Settings->BGMVolume = BGM;
	Settings->SFXVolume = SFX;
	Settings->UIVolume = UI;
	Settings->VoiceVolume = Voice;

	// Master × 세부 볼륨 (곱연산)
	// 사운드 믹스 시스템은 볼륨 값이 정확히 0.0f가 되는 순간,
	// 내부적으로 해당 사운드 클래스를 완전히 '비활성화(Mute)'시키거나
	// 페이드 타임 연산을 멈춰버리는 고질적인 버그성 특징
	// 예방을 위해 최소값(0.0001f)
	const float FinalMaster = FMath::Max(Master, 0.0001f);
	const float FinalBGM = FMath::Max(Master * BGM, 0.0001f);
	const float FinalSFX = FMath::Max(Master * SFX, 0.0001f);
	const float FinalUI = FMath::Max(Master * UI, 0.0001f);
	const float FinalVoice = FMath::Max(Master * Voice, 0.0001f);

	// Master 적용
	if (MasterSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			World,
			MasterSoundMix,
			MasterSoundClass,
			FinalMaster,
			1.0f,
			0.0f,
			true
		);
	}

	// BGM 적용
	if (BGMSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			World,
			MasterSoundMix,
			BGMSoundClass,
			FinalBGM,
			1.0f,
			0.0f,
			true
		);
	}

	// SFX 적용
	if (SFXSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			World,
			MasterSoundMix,
			SFXSoundClass,
			FinalSFX,
			1.0f,
			0.0f,
			true
		);
	}

	// UI 적용
	if (UISoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			World,
			MasterSoundMix,
			UISoundClass,
			FinalUI,
			1.0f,
			0.0f,
			true
		);
	}

	// Voice Input 적용
	if (VoiceSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			World,
			MasterSoundMix,
			VoiceSoundClass,
			FinalVoice,
			1.0f,
			0.0f,
			true
		);
	}
	
	UGameplayStatics::PushSoundMixModifier(World, MasterSoundMix);

	bIsDirty = true;
}

void UPTWOptionsWidget::UpdateInputSettings()
{
	UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());
	if (!Settings || !Slider_MouseSensitivity) return;

	if (!Slider_MouseSensitivity) return;

	float NewSensitivity = Slider_MouseSensitivity->GetValue();
	Settings->MouseSensitivity = NewSensitivity;

	if (APTWPlayerController* PC = Cast<APTWPlayerController>(GetOwningPlayer()))
	{
		PC->ApplyMouseSensitivity(NewSensitivity);
	}
	else if (APTWMainMenuPlayerController* MPC = Cast<APTWMainMenuPlayerController>(GetOwningPlayer()))
	{
		MPC->ApplyMouseSensitivity(NewSensitivity);
	}

	bIsDirty = true;
}

void UPTWOptionsWidget::UpdateScalabilitySettings()
{
	UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());
	if (!Settings) return;

	// 세부 품질 적용
	if (Combo_ViewDistance) Settings->SetViewDistanceQuality(Combo_ViewDistance->GetSelectedIndex());
	if (Combo_Shadow)       Settings->SetShadowQuality(Combo_Shadow->GetSelectedIndex());
	if (Combo_Texture)      Settings->SetTextureQuality(Combo_Texture->GetSelectedIndex());
	if (Combo_Effects)      Settings->SetVisualEffectQuality(Combo_Effects->GetSelectedIndex());
	if (Combo_PostProcess)  Settings->SetPostProcessingQuality(Combo_PostProcess->GetSelectedIndex());
	if (Combo_Foliage)      Settings->SetFoliageQuality(Combo_Foliage->GetSelectedIndex());
	if (Combo_Shading)      Settings->SetShadingQuality(Combo_Shading->GetSelectedIndex());
	if (Combo_AntiAliasing) Settings->SetAntiAliasingQuality(Combo_AntiAliasing->GetSelectedIndex());

	// Custom 여부 판별 로직 - 모든 세부 설정 값이 동일한지 확인
	int32 FirstVal = Settings->GetViewDistanceQuality();
	bool bIsCustom = false;

	TArray<int32> Qualities = {
		Settings->GetShadowQuality(),
		Settings->GetTextureQuality(),
		Settings->GetVisualEffectQuality(),
		Settings->GetPostProcessingQuality(),
		Settings->GetFoliageQuality(),
		Settings->GetShadingQuality(),
		Settings->GetAntiAliasingQuality()
	};

	for (int32 Val : Qualities)
	{
		if (Val != FirstVal)
		{
			bIsCustom = true;
			break;
		}
	}

	// 전체 품질 UI 업데이트
	if (Combo_Quality)
	{
		FString CustomText = NSLOCTEXT("Options", "Custom", "Custom").ToString();

		if (bIsCustom)
		{
			// 목록에 "Custom"이 없다면 추가
			if (Combo_Quality->FindOptionIndex(CustomText) == INDEX_NONE)
			{
				Combo_Quality->AddOption(CustomText);
			}
			Combo_Quality->SetSelectedOption(CustomText);
		}
		else
		{
			// 모든 값이 같아지면 "Custom" 옵션을 리스트에서 제거하여 선택 불가능하게 만들기
			Combo_Quality->RemoveOption(CustomText);

			Combo_Quality->SetSelectedIndex(FirstVal);
			Settings->SetOverallScalabilityLevel(FirstVal);
		}
	}

	// Settings->ApplySettings(false);
	bIsDirty = true;
}

void UPTWOptionsWidget::RestoreInitialSettings()
{
	UPTWGameUserSettings* Settings =
		Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());

	if (!Settings) return;

	// 디스플레이 복구
	Settings->SetScreenResolution(InitialSnapshot.Resolution);
	Settings->SetFullscreenMode(InitialSnapshot.WindowMode);

	// 프리셋 복구
	Settings->SetOverallScalabilityLevel(InitialSnapshot.ScalabilityLevel);

	// 세부 그래픽 옵션 복구
	Settings->SetViewDistanceQuality(InitialSnapshot.ViewDistanceQuality);
	Settings->SetShadowQuality(InitialSnapshot.ShadowQuality);
	Settings->SetTextureQuality(InitialSnapshot.TextureQuality);
	Settings->SetVisualEffectQuality(InitialSnapshot.EffectsQuality);
	Settings->SetPostProcessingQuality(InitialSnapshot.PostProcessQuality);
	Settings->SetFoliageQuality(InitialSnapshot.FoliageQuality);
	Settings->SetShadingQuality(InitialSnapshot.ShadingQuality);
	Settings->SetAntiAliasingQuality(InitialSnapshot.AntiAliasingQuality);
	Settings->SetVSyncEnabled(InitialSnapshot.bVSync);
	Settings->FieldOfView = InitialSnapshot.FieldOfView;
	Settings->MaxFrameRateIndex = InitialSnapshot.MaxFrameRateIndex;
	Settings->bMotionBlur = InitialSnapshot.bMotionBlur;

	if (FrameRateValues.IsValidIndex(Settings->MaxFrameRateIndex))
	{
		Settings->SetFrameRateLimit(FrameRateValues[Settings->MaxFrameRateIndex]);
	}

	// 오디오 복구
	Settings->MasterVolume = InitialSnapshot.MasterVolume;

	Settings->bIsPushToTalk = InitialSnapshot.bIsPushToTalk;
	
	// 마우스 감도 복구
	Settings->MouseSensitivity = InitialSnapshot.MouseSensitivity;

	// 크로스헤어 선택 복구
	Settings->SetCrosshairIndex(InitialSnapshot.CrosshairIndex);

	// 언어 복구
	Settings->SelectedLanguage = InitialSnapshot.SelectedLanguage;
	Settings->ApplyLanguageSettings();

	/* (옵션창 취소 시 되돌리기) */
	// 사운드 반영
	if (MasterSoundMix && MasterSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			GetWorld(),
			MasterSoundMix,
			MasterSoundClass,
			InitialSnapshot.MasterVolume,
			1.0f,
			0.0f,
			true
		);

		UGameplayStatics::PushSoundMixModifier(GetWorld(), MasterSoundMix);
	}

	// 마우스 감도 반영
	if (APTWPlayerController* PC = Cast<APTWPlayerController>(GetOwningPlayer()))
	{
		PC->ApplyMouseSensitivity(InitialSnapshot.MouseSensitivity);

		if (PC->PlayerCameraManager) PC->PlayerCameraManager->SetFOV(Settings->FieldOfView);
	}

	if (UWorld* World = GetWorld())
	{
		// 원래 모션블러 상태로 명령어 재실행
		UKismetSystemLibrary::ExecuteConsoleCommand(World, FString::Printf(TEXT("r.MotionBlurQuality %d"), Settings->bMotionBlur ? 4 : 0));
	}

	Settings->ApplySettings(false);
	InitializeUIFromCurrentSettings();
	bIsDirty = false;
}

void UPTWOptionsWidget::BindEvents()
{
	/* 그래픽 */
	if (CheckBox_Windowed)
	{
		CheckBox_Windowed->OnCheckStateChanged.AddDynamic(this, &UPTWOptionsWidget::OnCheckWindowedChanged);
	}

	if (Combo_Resolution)
	{
		Combo_Resolution->OnSelectionChanged.AddDynamic(this, &UPTWOptionsWidget::OnResolutionChanged);
	}

	if (Combo_Quality)
	{
		Combo_Quality->OnSelectionChanged.AddDynamic(this, &UPTWOptionsWidget::OnQualityChanged);
	}
	
	TArray<UComboBoxString*> DetailCombos = {
	Combo_ViewDistance, Combo_Shadow, Combo_Texture,
	Combo_Effects, Combo_PostProcess, Combo_Foliage,
	Combo_Shading, Combo_AntiAliasing
	};
	for (UComboBoxString* Combo : DetailCombos)
	{
		if (Combo)
		{
			// OnQualityChanged는 내부에서 UpdateScalabilitySettings()를 호출
			Combo->OnSelectionChanged.AddDynamic(this, &UPTWOptionsWidget::OnQualityChanged);
		}
	}

	if (CheckBox_VSync)
	{
		CheckBox_VSync->OnCheckStateChanged.AddDynamic(this, &UPTWOptionsWidget::OnVSyncChanged);
	}

	if (Slider_FOV) Slider_FOV->OnValueChanged.AddDynamic(this, &UPTWOptionsWidget::OnFOVChanged);
	if (ET_FOV) ET_FOV->OnTextCommitted.AddDynamic(this, &UPTWOptionsWidget::OnFOVTextCommitted);
	if (Combo_FrameLimit) Combo_FrameLimit->OnSelectionChanged.AddDynamic(this, &UPTWOptionsWidget::OnFrameLimitChanged);
	if (CheckBox_MotionBlur) CheckBox_MotionBlur->OnCheckStateChanged.AddDynamic(this, &UPTWOptionsWidget::OnMotionBlurChanged);

	 /* 사운드 */
	if (Slider_MasterVolume)
	{
		Slider_MasterVolume->OnValueChanged.AddDynamic(this, &UPTWOptionsWidget::OnMasterVolumeChanged);
	}
	if (ET_MasterVolume)
	{
		ET_MasterVolume->OnTextCommitted.AddDynamic(this, &UPTWOptionsWidget::OnMasterVolumeTextCommitted);
	}
	if (Slider_BGMVolume)
	{
		Slider_BGMVolume->OnValueChanged.AddDynamic(this, &UPTWOptionsWidget::OnBGMVolumeChanged);
	}
	if (ET_BGMVolume)
	{
		ET_BGMVolume->OnTextCommitted.AddDynamic(this, &UPTWOptionsWidget::OnBGMVolumeTextCommitted);
	}
	if (Slider_SFXVolume)
	{
		Slider_SFXVolume->OnValueChanged.AddDynamic(this, &UPTWOptionsWidget::OnSFXVolumeChanged);
	}
	if (ET_SFXVolume)
	{
		ET_SFXVolume->OnTextCommitted.AddDynamic(this, &UPTWOptionsWidget::OnSFXVolumeTextCommitted);
	}
	if (Slider_UIVolume)
	{
		Slider_UIVolume->OnValueChanged.AddDynamic(this, &UPTWOptionsWidget::OnUIVolumeChanged);
	}
	if (ET_UIVolume)
	{
		ET_UIVolume->OnTextCommitted.AddDynamic(this, &UPTWOptionsWidget::OnUIVolumeTextCommitted);
	}
	if (Slider_VoiceVolume)
	{
		Slider_VoiceVolume->OnValueChanged.AddDynamic(this, &UPTWOptionsWidget::OnVoiceVolumeChanged);
	}
	if (ET_VoiceVolume)
	{
		ET_VoiceVolume->OnTextCommitted.AddDynamic(this, &UPTWOptionsWidget::OnVoiceVolumeTextCommitted);
	}
	
	/* 게임 세팅 */
	if (Slider_MouseSensitivity)
	{
		// 드래그 중에는 값만 업데이트
		Slider_MouseSensitivity->OnValueChanged.AddDynamic(this, &UPTWOptionsWidget::OnMouseSensitivityChanged);

		// 마우스를 뗄 때 엔진에 최종 적용
		Slider_MouseSensitivity->OnMouseCaptureEnd.AddDynamic(this, &UPTWOptionsWidget::OnSensitivityCaptureEnd);
	}
	if (ET_MouseSensitivity)
	{
		ET_MouseSensitivity->OnTextCommitted.AddDynamic(this, &UPTWOptionsWidget::OnMouseSensitivityTextCommitted);
	}
	if (CheckBox_InvertY)
	{
		CheckBox_InvertY->OnCheckStateChanged.AddDynamic(this, &UPTWOptionsWidget::OnInvertYChanged);
	}

	/* 카테고리 버튼 */
	if (Button_Graphics)
	{
		Button_Graphics->OnClicked.AddDynamic(
			this, &UPTWOptionsWidget::OnClickedGraphics);
	}
	if (Button_Sound)
	{
		Button_Sound->OnClicked.AddDynamic(
			this, &UPTWOptionsWidget::OnClickedSound);
	}
	if (Button_Game)
	{
		Button_Game->OnClicked.AddDynamic(
			this, &UPTWOptionsWidget::OnClickedGame);
	}

	/* 버튼 */
	if (Button_Save)
	{
		Button_Save->OnClicked.AddDynamic(
			this, &UPTWOptionsWidget::OnClickedSave);
	}
	if (Button_Cancel)
	{
		Button_Cancel->OnClicked.AddDynamic(
			this, &UPTWOptionsWidget::OnClickedCancel);
	}
	/* Help 버튼 */
	if (Button_VoiceHelp)
	{
		Button_VoiceHelp->OnClicked.AddDynamic(
			this, &UPTWOptionsWidget::OnClickedVoiceHelp);
	}
	
	/* 언어 설정 */
	if (Combo_Language)
	{
		Combo_Language->OnSelectionChanged.AddDynamic(this, &UPTWOptionsWidget::OnLanguageChanged);
	}
}

bool UPTWOptionsWidget::ValidateAndApplyTextEntry(const FText& InText, USlider* TargetSlider, UEditableText* TargetET, float MinValue, float MaxValue)
{
	if (!TargetSlider || !TargetET) return false;

	FString StringValue = InText.ToString();

	// 공백 체크
	if (StringValue.IsEmpty())
	{
		TargetET->SetText(FormatFloatToText(TargetSlider->GetValue() * 100.f, true));
		return false;
	}

	// 숫자 변환 시도
	float NewValue = FCString::Atof(*StringValue);

	// 예외 케이스 처리: 변환값이 0인데 입력이 "0"이 아닌 경우 (즉, 문자 입력)
	if (NewValue == 0.0f && !StringValue.Equals(TEXT("0")) && !StringValue.Equals(TEXT("0.0")))
	{
		// 잘못된 입력일 경우 현재 슬라이더의 실제 값을 가져와 텍스트 박스에 다시 덮어씌움
		TargetET->SetText(FormatFloatToText(TargetSlider->GetValue() * 100.f, true));
		return false;
	}

	// 유효한 입력인 경우: 범위 제한 후 슬라이더 및 텍스트 업데이트
	NewValue = FMath::Clamp(NewValue, MinValue, MaxValue);

	float SliderValue = NewValue / 100.f;
	TargetSlider->SetValue(SliderValue);

	// 입력을 포맷에 맞춰 재출력 (예: 사용자가 0.5555 입력 시 0.56으로 보정 표시)
	TargetET->SetText(FormatFloatToText(NewValue, true));

	return true;
}

FText UPTWOptionsWidget::FormatFloatToText(float Value, bool bInt) const
{
	FNumberFormattingOptions NumberOptions = FNumberFormattingOptions::DefaultNoGrouping();
	
	if (bInt)
	{
		NumberOptions.SetMaximumFractionalDigits(0);
		NumberOptions.SetMinimumFractionalDigits(0);
	}
	else
	{
		NumberOptions.SetMaximumFractionalDigits(2);
		NumberOptions.SetMinimumFractionalDigits(2);
	}

	return FText::AsNumber(Value, &NumberOptions);
}

void UPTWOptionsWidget::OnCheckWindowedChanged(bool bChecked)
{
	UpdateDisplaySettings();
}

void UPTWOptionsWidget::OnResolutionChanged(
	FString SelectedItem,
	ESelectInfo::Type SelectionType)
{
	// 초기화 과정에서 호출되는 것 방지
	if (SelectionType == ESelectInfo::Direct) return;

	UpdateDisplaySettings();
}

void UPTWOptionsWidget::OnQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct) return;

	// "Custom" 텍스트가 들어온 경우 로직을 수행하지 않고 리턴
	if (SelectedItem == NSLOCTEXT("Options", "Custom", "Custom").ToString())
	{
		return;
	}

	// 사용자가 전체 품질 콤보박스를 직접 건드린 경우
	if (Combo_Quality && Combo_Quality->GetSelectedOption() == SelectedItem)
	{
		int32 NewIndex = Combo_Quality->GetSelectedIndex();
		if (NewIndex != -1)
		{
			if (Combo_ViewDistance) Combo_ViewDistance->SetSelectedIndex(NewIndex);
			if (Combo_Shadow)       Combo_Shadow->SetSelectedIndex(NewIndex);
			if (Combo_Texture)      Combo_Texture->SetSelectedIndex(NewIndex);
			if (Combo_Effects)      Combo_Effects->SetSelectedIndex(NewIndex);
			if (Combo_PostProcess)  Combo_PostProcess->SetSelectedIndex(NewIndex);
			if (Combo_Foliage)      Combo_Foliage->SetSelectedIndex(NewIndex);
			if (Combo_Shading)      Combo_Shading->SetSelectedIndex(NewIndex);
			if (Combo_AntiAliasing) Combo_AntiAliasing->SetSelectedIndex(NewIndex);
		}
	}

	// 만약 세부 항목 콤보박스를 건드렸다면 바로 엔진에 적용
	UpdateScalabilitySettings();
}

void UPTWOptionsWidget::OnVSyncChanged(bool bChecked)
{
	UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());
	if (!Settings) return;

	Settings->SetVSyncEnabled(bChecked);
	bIsDirty = true;
}

void UPTWOptionsWidget::OnFOVChanged(float Value)
{
	if (ET_FOV) ET_FOV->SetText(FormatFloatToText(Value));

	if (UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings()))
	{
		Settings->FieldOfView = Value;
		bIsDirty = true;
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->SetFOV(Value);
		}
	}
}

void UPTWOptionsWidget::OnFOVTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// 최소 시야각 70, 최대 시야각 110 제한 
	if (ValidateAndApplyTextEntry(Text, Slider_FOV, ET_FOV, 70.0f, 110.0f))
	{
		OnFOVChanged(Slider_FOV->GetValue());
	}
}

void UPTWOptionsWidget::OnFrameLimitChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct) return;

	int32 SelectedIdx = Combo_FrameLimit->GetSelectedIndex();
	if (FrameRateValues.IsValidIndex(SelectedIdx))
	{
		if (UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings()))
		{
			Settings->MaxFrameRateIndex = SelectedIdx;
			Settings->SetFrameRateLimit(FrameRateValues[SelectedIdx]);
			Settings->ApplySettings(false);
			bIsDirty = true;
		}
	}
}

void UPTWOptionsWidget::OnMotionBlurChanged(bool bChecked)
{
	if (UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings()))
	{
		Settings->bMotionBlur = bChecked;

		if (UWorld* World = GetWorld())
		{
			UKismetSystemLibrary::ExecuteConsoleCommand(World, FString::Printf(TEXT("r.MotionBlurQuality %d"), bChecked ? 4 : 0));
		}
		bIsDirty = true;
	}
}

void UPTWOptionsWidget::OnMasterVolumeChanged(float Value)
{
	if (ET_MasterVolume) ET_MasterVolume->SetText(FormatFloatToText(Value * 100.f, true));

	UpdateAudioSettings();
}

void UPTWOptionsWidget::OnMasterVolumeTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (ValidateAndApplyTextEntry(Text, Slider_MasterVolume, ET_MasterVolume, 0.0f, 100.0f))
	{
		UpdateAudioSettings();
	}
}

void UPTWOptionsWidget::OnBGMVolumeChanged(float Value)
{
	if (ET_BGMVolume) ET_BGMVolume->SetText(FormatFloatToText(Value * 100.f, true));
	UpdateAudioSettings();
}

void UPTWOptionsWidget::OnBGMVolumeTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (ValidateAndApplyTextEntry(Text, Slider_BGMVolume, ET_BGMVolume, 0.0f, 100.0f))
	{
		UpdateAudioSettings();
	}
}

void UPTWOptionsWidget::OnSFXVolumeChanged(float Value)
{
	if (ET_SFXVolume) ET_SFXVolume->SetText(FormatFloatToText(Value * 100.f, true));
	UpdateAudioSettings();
}

void UPTWOptionsWidget::OnSFXVolumeTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (ValidateAndApplyTextEntry(Text, Slider_SFXVolume, ET_SFXVolume, 0.0f, 100.0f))
	{
		UpdateAudioSettings();
	}
}

void UPTWOptionsWidget::OnUIVolumeChanged(float Value)
{
	if (ET_UIVolume) ET_UIVolume->SetText(FormatFloatToText(Value * 100.f, true));
	UpdateAudioSettings();
}

void UPTWOptionsWidget::OnUIVolumeTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (ValidateAndApplyTextEntry(Text, Slider_UIVolume, ET_UIVolume, 0.0f, 100.0f))
	{
		UpdateAudioSettings();
	}
}

void UPTWOptionsWidget::OnVoiceVolumeChanged(float Value)
{
	if (ET_VoiceVolume) ET_VoiceVolume->SetText(FormatFloatToText(Value * 100.f, true));
	UpdateAudioSettings();
}

void UPTWOptionsWidget::OnVoiceVolumeTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (ValidateAndApplyTextEntry(Text, Slider_VoiceVolume, ET_VoiceVolume, 0.0f, 100.0f))
	{
		UpdateAudioSettings();
	}
}

void UPTWOptionsWidget::OnMouseSensitivityChanged(float Value)
{
	if (ET_MouseSensitivity)
	{
		ET_MouseSensitivity->SetText(FormatFloatToText(Value));
	}

	if (APTWPlayerController* PC = Cast<APTWPlayerController>(GetOwningPlayer()))
	{
		PC->ApplyMouseSensitivity(Value);
	}
}

void UPTWOptionsWidget::OnSensitivityCaptureEnd()
{
	UpdateInputSettings();
}

void UPTWOptionsWidget::OnMouseSensitivityTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// 숫자 검증 및 동기화 (0.0 ~ 5.0 범위)
	if (ValidateAndApplyTextEntry(Text, Slider_MouseSensitivity, ET_MouseSensitivity, 0.0f, 5.0f))
	{
		// 유효한 입력이었을 때만 엔진 설정 업데이트
		UpdateInputSettings();
	}
}

void UPTWOptionsWidget::OnInvertYChanged(bool bIsChecked)
{
	UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());
	if (!Settings) return;

	Settings->bInvertYAxis = bIsChecked;
	bIsDirty = true;
}

void UPTWOptionsWidget::OnLanguageChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct) return;

	UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());
	if (Settings && LanguageMap.Contains(SelectedItem))
	{
		FString NewCultureCode = LanguageMap[SelectedItem];
		Settings->SelectedLanguage = NewCultureCode;

		// 즉시 반영
		Settings->ApplyLanguageSettings();

		bIsDirty = true;
	}
}

void UPTWOptionsWidget::OnClickedSave()
{
	UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());

	UE_LOG(LogTemp, Error, TEXT("OptionWidget : Save1."));

	if (!Settings) return;

	UE_LOG(LogTemp, Error, TEXT("OptionWidget : Save2."));
	
	FString PushToTalk = NSLOCTEXT("Options", "PushToTalk", "PushToTalk").ToString();
	bool bIsPushToTalk = Combo_VoiceInputMode->GetSelectedOption() == PushToTalk ? true : false;
	if (Settings->bIsPushToTalk != bIsPushToTalk)
	{
		if (UPTWVoiceChatSubsystem* VoiceChatSubsystem = UPTWVoiceChatSubsystem::Get(this))
		{
			VoiceChatSubsystem->SetVoiceInputMode(bIsPushToTalk);
			Settings->bIsPushToTalk = bIsPushToTalk;
		}
	}
	
	Settings->SetCrosshairIndex(CurrentSelectedCrosshairIndex);

	Settings->ApplySettings(false);
	Settings->SaveSettings();

	CacheInitialSettings();

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
		{
			UISubsystem->PopWidget();
		}
	}
}

void UPTWOptionsWidget::OnClickedCancel()
{
	if (bIsDirty)
	{
		RestoreInitialSettings();
	}

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
		{
			UISubsystem->PopWidget();
		}
	}
}

void UPTWOptionsWidget::OnClickedVoiceHelp()
{
	APlayerController* PC = GetOwningPlayer();
	
	FText VoiceHelpText = FText::FromString(TEXT("스팀 오버레이(Shift + Tab)를 열고 [설정] -> [음성] 탭에서 마이크 입력장치 변경 및 볼륨조절이 가능합니다"));
	if (APTWPlayerController* PTWPC = Cast<APTWPlayerController>(PC))
	{
		PTWPC->Popup(VoiceHelpText);
	}
	else if (APTWMainMenuPlayerController* PTWMainPC = Cast<APTWMainMenuPlayerController>(PC))
	{
		PTWMainPC->Popup(VoiceHelpText);
	}
}

void UPTWOptionsWidget::OnClickedGraphics()
{
	if (CategorySwitcher)
		CategorySwitcher->SetActiveWidgetIndex(0);
}

void UPTWOptionsWidget::OnClickedSound()
{
	if (CategorySwitcher)
		CategorySwitcher->SetActiveWidgetIndex(1);
}

void UPTWOptionsWidget::OnClickedGame()
{
	if (CategorySwitcher)
		CategorySwitcher->SetActiveWidgetIndex(2);
}

void UPTWOptionsWidget::BuildCrosshairListDirect()
{
	if (!CrosshairHorizontalBox || !CrosshairDataTable) return;

	// 기존 자식 위젯들과 제어용 보더 배열 깔끔하게 비우기
	CrosshairHorizontalBox->ClearChildren();
	CreatedSlotBorders.Empty();

	TArray<FCrosshairData*> AllRows;
	CrosshairDataTable->GetAllRows<FCrosshairData>(TEXT(""), AllRows);

	for (int32 i = 0; i < AllRows.Num(); ++i)
	{
		FCrosshairData* RowData = AllRows[i];
		if (!RowData) continue;

		// 1. 크기 고정용 SizeBox 생성 (120x120)
		USizeBox* NewSizeBox = NewObject<USizeBox>(this);
		if (!NewSizeBox) continue;
		NewSizeBox->SetWidthOverride(120.f);
		NewSizeBox->SetHeightOverride(120.f);

		// 2. 하이라이트 테두리용 Border 생성
		UBorder* NewBorder = NewObject<UBorder>(this);
		if (!NewBorder) continue;

		// 선택된 인덱스면 초록색 테두리, 아니면 투명 처리
		FLinearColor InitBorderColor = (i == CurrentSelectedCrosshairIndex) ? FLinearColor::Green : FLinearColor::Transparent;
		NewBorder->SetBrushColor(InitBorderColor);
		NewBorder->SetPadding(FMargin(4.f)); // 테두리 두께

		// 3. 클릭 처리용 투명 Button 생성
		UButton* NewButton = NewObject<UButton>(this);
		if (!NewButton) continue;

		// 버튼의 기존 외형 텍스처를 제거하여 완전 투명화
		NewButton->WidgetStyle.Normal.DrawAs = ESlateBrushDrawType::NoDrawType;
		NewButton->WidgetStyle.Hovered.DrawAs = ESlateBrushDrawType::NoDrawType;
		NewButton->WidgetStyle.Pressed.DrawAs = ESlateBrushDrawType::NoDrawType;

		NewButton->OnClicked.AddDynamic(this, &UPTWOptionsWidget::OnCrosshairSlotClicked);

		// 4. 데이터테이블 WBP 에셋 로드 및 생성
		UClass* WidgetClass = RowData->CrosshairWidgetClass.LoadSynchronous();
		if (WidgetClass)
		{
			UUserWidget* CrosshairWBP = CreateWidget<UUserWidget>(GetOwningPlayer(), WidgetClass);
			if (CrosshairWBP)
			{
				// UI 트리 계층 구조 조립: WBP -> Button -> Border -> SizeBox
				NewButton->AddChild(CrosshairWBP);
				NewBorder->AddChild(NewButton);
				NewSizeBox->AddChild(NewBorder);

				// 스크롤 상자나 가로 상자에 최종 등록
				CrosshairHorizontalBox->AddChild(NewSizeBox);

				// 클릭 시 실시간 하이라이트 제어를 위해 보더 주소 기억
				CreatedSlotBorders.Add(NewBorder);
			}
		}
	}
}

void UPTWOptionsWidget::OnCrosshairSlotClicked()
{
	int32 ClickedIndex = INDEX_NONE;

	// 모든 슬롯을 순회하며 어떤 버튼이 "실제 포커스나 클릭된 상태"인지 찾습니다.
	for (int32 i = 0; i < CreatedSlotBorders.Num(); ++i)
	{
		if (UBorder* Border = CreatedSlotBorders[i])
		{
			if (UButton* Button = Cast<UButton>(Border->GetChildAt(0)))
			{
				if (Button->IsHovered() || Button->HasAnyUserFocus() || Button->IsPressed())
				{
					ClickedIndex = i;
					break;
				}
			}
		}
	}

	// 만약 마우스 포커스 타이밍 문제로 위 조건이 안 잡힐 때를 대비한 2차 방어선 (배열 기반 검사)
	if (ClickedIndex == INDEX_NONE)
	{
		for (int32 i = 0; i < CreatedSlotBorders.Num(); ++i)
		{
			if (CreatedSlotBorders[i] && CreatedSlotBorders[i]->IsHovered())
			{
				ClickedIndex = i;
				break;
			}
		}
	}

	// 디버깅용 로그 (출력 창이나 인게임 `~` 콘솔창에서 확인 가능)
	UE_LOG(LogTemp, Log, TEXT("[Crosshair] Clicked Index: %d"), ClickedIndex);

	if (ClickedIndex == INDEX_NONE || CurrentSelectedCrosshairIndex == ClickedIndex) return;

	CurrentSelectedCrosshairIndex = ClickedIndex;
	bIsDirty = true; // 변경 사항이 생겼으므로 세이브/캔슬 활성화

	// 테두리 하이라이트 실시간 갱신
	for (int32 i = 0; i < CreatedSlotBorders.Num(); ++i)
	{
		if (CreatedSlotBorders[i])
		{
			FLinearColor NewColor = (i == CurrentSelectedCrosshairIndex) ? FLinearColor::Green : FLinearColor::Transparent;
			CreatedSlotBorders[i]->SetBrushColor(NewColor);
		}
	}
}
