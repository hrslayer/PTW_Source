// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "PTWGameUserSettings.generated.h"

// 크로스헤어 변경을 인게임 UI에 즉시 전파하기 위한 C++ 델리게이트 선언
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCrosshairChangedSignature, int32);

/**
 * 
 */
UCLASS()
class PTW_API UPTWGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()
	
public:
	// 사용자가 설정한 오디오값 적용 (게임 재시작 같은 상황 시 Instance 에서 호출하여 저장값 적용)
	void ApplyAudioSettings(UWorld* World, USoundMix* SoundMix,
		USoundClass* MasterClass,
		USoundClass* BGMClass,
		USoundClass* SFXClass,
		USoundClass* UIClass,
		USoundClass* VoiceClass);

	// 언어 적용 함수
	void ApplyLanguageSettings();

	UPROPERTY(config)
	float FieldOfView = 90.f; // 기본 시야각

	UPROPERTY(config)
	int32 MaxFrameRateIndex = 2; // 0=무제한, 1=60, 2=144, 3=240 등 (콤보박스 인덱스 대응)

	UPROPERTY(config)
	bool bInvertYAxis = false;

	UPROPERTY(config)
	bool bMotionBlur = false; 

	UPROPERTY(config)
	float MasterVolume = 1.f;

	UPROPERTY(Config)
	float BGMVolume = 1.f;

	UPROPERTY(Config)
	float SFXVolume = 1.f;

	UPROPERTY(Config)
	float UIVolume = 1.f;

	UPROPERTY(Config)
	float VoiceVolume = 1.f;
	
	UPROPERTY(config)
	float MouseSensitivity = 1.f;

	UPROPERTY(config)
	bool bIsPushToTalk = true;
	
	UPROPERTY(config)
	TMap<FString, float> PlayerVolumes;
	
	/* 언어 저장값 (디폴트: 한국어) */
	UPROPERTY(Config)
	FString SelectedLanguage = TEXT("ko");

	/* KeyGuide On/Off */
	void SetbKeyGuideOn(bool KeyGuide);
	FORCEINLINE bool GetbKeyGuideOn() { return bKeyGuideOn; }

	/** 크로스헤어 인터페이스 */
	FOnCrosshairChangedSignature OnCrosshairChanged;

	void SetCrosshairIndex(int32 InIndex)
	{
		if (CrosshairIndex != InIndex)
		{
			CrosshairIndex = InIndex;
			OnCrosshairChanged.Broadcast(CrosshairIndex);
		}
	}
	int32 GetCrosshairIndex() const { return CrosshairIndex; }

private:
	bool bKeyGuideOn = true;

	UPROPERTY(config)
	int32 CrosshairIndex = 0;
};
