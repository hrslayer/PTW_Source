// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameUserSettings.h"
#include "PTWOptionsWidget.generated.h"

class UComboBoxKey;
class UVerticalBox;
class UCheckBox;
class UComboBoxString;
class USlider;
class UButton;
class UWidgetSwitcher;
class UHorizontalBox;
class UDataTable;
class UBorder;
class USoundClass;
class USoundMix;
class UEditableText;
class UPTWVoiceVolume;

USTRUCT()
struct FOptionSnapshot
{
	GENERATED_BODY()

	FIntPoint Resolution;
	EWindowMode::Type WindowMode;
	int32 ScalabilityLevel;
	
	int32 ViewDistanceQuality;
	int32 ShadowQuality;
	int32 TextureQuality;
	int32 EffectsQuality;
	int32 PostProcessQuality;
	int32 FoliageQuality;
	int32 ShadingQuality;
	int32 AntiAliasingQuality;
	bool bVSync;
	float FieldOfView = 90.f;
	int32 MaxFrameRateIndex = 2;
	bool bMotionBlur = false;

	float MasterVolume = 1.f;
	float BGMVolume = 1.f;
	float SFXVolume = 1.f;
	float UIVolume = 1.f;
	float VoiceVolume = 1.f;

	float MouseSensitivity = 1.f;
	bool bInvertYAxis = false;
	int32 CrosshairIndex = 0;

	bool bIsPushToTalk = true;
	FString SelectedLanguage;
};
/**
 * 
 */
UCLASS()
class PTW_API UPTWOptionsWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/* UI 바인딩 */
	// 창모드
	UPROPERTY(meta = (BindWidget))
	UCheckBox* CheckBox_Windowed;
	// 해상도
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_Resolution;
	// 그래픽 품질
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_Quality;
	// 그래픽 품질 세부사항
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_ViewDistance; // 시야 거리
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_Shadow; // 그림자
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_Texture; // 텍스쳐
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_Effects; // 이펙트
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_PostProcess; // 후처리
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_Foliage; // 식생
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_Shading; // 셰이딩
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_AntiAliasing; // 안티앨리어싱
	UPROPERTY(meta = (BindWidget))
	UCheckBox* CheckBox_VSync; // 수직동기화
	UPROPERTY(meta = (BindWidget))
	USlider* Slider_FOV; // 시야각
	UPROPERTY(meta = (BindWidget))
	UEditableText* ET_FOV; 
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_FrameLimit; // 프레임제한
	UPROPERTY(meta = (BindWidget))
	UCheckBox* CheckBox_MotionBlur; // 모션블러
	// 마스터볼륨
	UPROPERTY(meta = (BindWidget))
	USlider* Slider_MasterVolume;
	UPROPERTY(meta = (BindWidget))
	UEditableText* ET_MasterVolume;
	// 사운드 세부사항
	UPROPERTY(meta = (BindWidget)) //BGM
	USlider* Slider_BGMVolume;
	UPROPERTY(meta = (BindWidget))
	UEditableText* ET_BGMVolume;
	UPROPERTY(meta = (BindWidget)) // SFX
	USlider* Slider_SFXVolume;
	UPROPERTY(meta = (BindWidget))
	UEditableText* ET_SFXVolume;
	UPROPERTY(meta = (BindWidget)) // UI
	USlider* Slider_UIVolume;
	UPROPERTY(meta = (BindWidget))
	UEditableText* ET_UIVolume;
	UPROPERTY(meta = (BindWidget)) // Voice
	USlider* Slider_VoiceVolume;
	UPROPERTY(meta = (BindWidget))
	UEditableText* ET_VoiceVolume;
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* Vertical_VoiceVolume;
	// 게임세팅
	UPROPERTY(meta = (BindWidget))
	USlider* Slider_MouseSensitivity; // 마우스 감도
	UPROPERTY(meta = (BindWidget))
	UEditableText* ET_MouseSensitivity;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> CheckBox_InvertY; // 마우스 상하반전
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UHorizontalBox> CrosshairHorizontalBox; // 크로스헤어 선택

	// 세이브 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Save;
	// 캔슬 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Cancel;
	// 언어 설정
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_Language;

	/* 카테고리 UI */
	// 카테고리 스위치
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* CategorySwitcher;
	// 그래픽 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Graphics;
	// 사운드 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Sound;
	// 게임설정 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Game;
	// 보이스볼륨 도움말 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* Button_VoiceHelp;
	// 보이스 입력 모드 변경 선택지
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_VoiceInputMode;
	
	/* 에디터에서 할당할 SoundMix */
	UPROPERTY(EditAnywhere, Category = "Settings|Sound")
	USoundMix* MasterSoundMix;
	/* 에디터에서 할당할 SoundClass */
	UPROPERTY(EditAnywhere, Category = "Settings|Sound")
	USoundClass* MasterSoundClass;
	UPROPERTY(EditAnywhere, Category = "Settings|Sound")
	USoundClass* BGMSoundClass;
	UPROPERTY(EditAnywhere, Category = "Settings|Sound")
	USoundClass* SFXSoundClass;
	UPROPERTY(EditAnywhere, Category = "Settings|Sound")
	USoundClass* UISoundClass;
	UPROPERTY(EditAnywhere, Category = "Settings|Sound")
	USoundClass* VoiceSoundClass;
	/* 에디터에서 할당할 크로스헤어 목록 데이터 테이블 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Options | Crosshair")
	TObjectPtr<UDataTable> CrosshairDataTable;

private:
	FOptionSnapshot InitialSnapshot; // 초기값 저장
	bool bIsDirty = false; // 설정 변경 여부

	/* 내부 로직 */

	void PopulateResolutionList();
	void PopulateQualityList();
	void PopulateLanguageList();
	void PopulateVoiceInputModeList();
	void PopulateFrameLimitList();

	void SetupLanguageData();
	void InitializeUIFromCurrentSettings();
	void CacheInitialSettings();

	void UpdateDisplaySettings(); // 해상도, 창모드, VSync
	void UpdateAudioSettings();   // 볼륨 전용
	void UpdateInputSettings();   // 감도 전용
	void UpdateScalabilitySettings(); // 그래픽 세부 항목 전용
	void RestoreInitialSettings();

	void BindEvents();
	// 텍스트 입력값을 검증하고 슬라이더 및 텍스트 박스를 동기화하는 공통 함수
	bool ValidateAndApplyTextEntry(const FText& InText, USlider* TargetSlider, UEditableText* TargetET, float MinValue, float MaxValue);
	// 숫자 포맷팅
	FText FormatFloatToText(float Value, bool bInt = false) const;

	/* 설정 변경 이벤트 */

	UFUNCTION()
	void OnCheckWindowedChanged(bool bChecked);

	UFUNCTION()
	void OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnVSyncChanged(bool bChecked);

	UFUNCTION()
	void OnFOVChanged(float Value);

	UFUNCTION()
	void OnFOVTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnFrameLimitChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnMotionBlurChanged(bool bChecked);

	UFUNCTION()
	void OnMasterVolumeChanged(float Value);
	UFUNCTION()
	void OnMasterVolumeTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	UFUNCTION()
	void OnBGMVolumeChanged(float Value);
	UFUNCTION()
	void OnBGMVolumeTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	UFUNCTION()
	void OnSFXVolumeChanged(float Value);
	UFUNCTION()
	void OnSFXVolumeTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	UFUNCTION()
	void OnUIVolumeChanged(float Value);
	UFUNCTION()
	void OnUIVolumeTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	UFUNCTION()
	void OnVoiceVolumeChanged(float Value);
	UFUNCTION()
	void OnVoiceVolumeTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
	UFUNCTION()
	void OnMouseSensitivityChanged(float Value);
	UFUNCTION()
	void OnSensitivityCaptureEnd();
	UFUNCTION()
	void OnMouseSensitivityTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	UFUNCTION()
	void OnInvertYChanged(bool bIsChecked);

	UFUNCTION()
	void OnLanguageChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnClickedSave();

	UFUNCTION()
	void OnClickedCancel();

	UFUNCTION()
	void OnClickedVoiceHelp();
	
	/* 카테고리 전환 */

	UFUNCTION()
	void OnClickedGraphics();

	UFUNCTION()
	void OnClickedSound();

	UFUNCTION()
	void OnClickedGame();

	/* 크로스헤어 */
	// 리스트 빌드
	void BuildCrosshairListDirect();
	// 내부 보더(또는 버튼)가 클릭되었을 때 호출할 함수
	UFUNCTION()
	void OnCrosshairSlotClicked();
	// 현재 옵션 창에서 임시 선택 중인 크로스헤어 인덱스
	int32 CurrentSelectedCrosshairIndex = 0;
	// 테두리(하이라이트) 색상을 실시간으로 껐다 켜기 위해 생성된 보더들을 보관하는 배열
	UPROPERTY()
	TArray<TObjectPtr<UBorder>> CreatedSlotBorders;

	// 언어 설정에서 코드와 이름을 매핑하기 위한 맵
	TMap<FString, FString> LanguageMap;

	// 프레임 레이트 값 매핑 리스트
	TArray<float> FrameRateValues;
};
