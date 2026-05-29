#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"

#include "ModularNames.h"
#include "ModularPlayerState.h"
#include "MoviePlayer.h"
#include "Blueprint/UserWidget.h"
#include "Components/GameFrameworkComponentManager.h"
#include "MiniGame/PTWMiniGameMapRow.h"
#include "UI/LoadingScreen/PTWLoadingMiniGame.h"
#include "UI/LoadingScreen/PTWLoadingWidgetBase.h"
#include "CoreFramework/PTWGameUserSettings.h"
#include "CoreFramework/PTWPlayerState.h"
#include "System/PTWSteamSessionSubsystem.h"


UPTWGameInstance::UPTWGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NextLoadingType = ELoadingScreenType::None;
}
void UPTWGameInstance::Init()
{
	Super::Init();

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UPTWGameInstance::BeginLoadingScreen);

	/* 사운드 저장값 자동 적용 */
	if (!GEngine) return;

	UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());
	if (!Settings) return;

	// 저장된 설정 로드
	Settings->LoadSettings(false);

	// 오디오 적용
	Settings->ApplyAudioSettings(
		GetWorld(),
		MasterSoundMix,
		MasterSoundClass,
		BGMSoundClass,
		SFXSoundClass,
		UISoundClass,
		VoiceSoundClass
	);
	
	if (UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>())
	{
		PlayerStateExtensionHandle = ComponentManager->AddExtensionHandler(
			APTWPlayerState::StaticClass(),
			UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandlePlayerStateExtension));
	}
}

void UPTWGameInstance::Shutdown()
{
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	PlayerStateExtensionHandle.Reset();
	
	Super::Shutdown();
}

void UPTWGameInstance::HandlePlayerStateExtension(AActor* Receiver, FName EventName)
{
	if (APTWPlayerState* PS = Cast<APTWPlayerState>(Receiver))
	{
		const FString UniqueId = PS->GetUniqueId().IsValid() ? PS->GetUniqueId().ToString() : TEXT("");
		const FString PlayerName = PS->GetPlayerName();
		
		if (EventName.IsEqual(ModularNames::NAME_ReplicatedActorReady))
		{
			if (!LevelPlayerIds.Contains(UniqueId))
			{
				LevelPlayerIds.Add(UniqueId);
			}
			OnLevelPlayerConnected.Broadcast(UniqueId);
			
			if (!SessionPlayerIds.Contains(UniqueId))
			{
				SessionPlayerIds.Add(UniqueId);
			}
			if (!SessionPlayerNames.Contains(UniqueId))
			{
				SessionPlayerNames.Add(UniqueId, FString());
			}
			SessionPlayerNames[UniqueId] = PlayerName;
			OnSessionPlayerConnected.Broadcast(UniqueId);
		}
		else if (EventName.IsEqual(UGameFrameworkComponentManager::NAME_ReceiverRemoved))
		{
			LevelPlayerIds.Remove(UniqueId);
			OnLevelPlayerDisconnected.Broadcast(UniqueId);

			SessionPlayerIds.Remove(UniqueId);
			SessionPlayerNames.Remove(UniqueId);
			OnSessionPlayerDisconnected.Broadcast(UniqueId);
		}
	}
}

void UPTWGameInstance::PrepareLoadingScreen(ELoadingScreenType InType, FName InMapRowName)
{
	NextLoadingType = InType;
	TargetMapRowName = InMapRowName;
}

void UPTWGameInstance::BeginLoadingScreen(const FString& MapName)
{
	DisplayLoadingScreen();
}

void UPTWGameInstance::DisplayLoadingScreen()
{
	if (IsRunningDedicatedServer()) return;

	if (NextLoadingType == ELoadingScreenType::None)
	{
		return;
	}

	// 위젯 클래스 결정
	TSubclassOf<UPTWLoadingWidgetBase> TargetClass = (NextLoadingType == ELoadingScreenType::Lobby)
		? LobbyLoadingWidgetClass
		: MiniGameLoadingWidgetClass;

	if (!TargetClass)
	{
		return;
	}
	// 위젯 생성 (타입을 구체적으로 지정)
	UPTWLoadingWidgetBase* LoadingWidget = CreateWidget<UPTWLoadingWidgetBase>(this, TargetClass);
	if (!LoadingWidget) return;

	// 데이터 세팅
	if (NextLoadingType == ELoadingScreenType::MiniGame && MiniGameMapTable)
	{
		static const FString ContextString(TEXT("LoadingContext"));
		FPTWMiniGameMapRow* MapData = MiniGameMapTable->FindRow<FPTWMiniGameMapRow>(TargetMapRowName, ContextString);

		if (MapData)
		{
			// 공통 이미지 세팅 (UPTWLoadingWidgetBase의 함수)
			LoadingWidget->InitBaseUI(MapData->Thumbnail);

			// 미니게임 전용 텍스트 세팅 (자식으로 캐스팅하여 호출)
			if (UPTWLoadingMiniGame* MiniGameWidget = Cast<UPTWLoadingMiniGame>(LoadingWidget))
			{
				MiniGameWidget->SetupMiniGameInfo(MapData->DisplayName, MapData->MapDescription);
			}
		}
	}
	else // 로비일 때
	{
		// 클래스에 추가한 LobbyDefaultImage 사용
		LoadingWidget->InitBaseUI(LobbyDefaultImage);
	}

	// MoviePlayer 등록
	FLoadingScreenAttributes LoadingScreen;
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
	LoadingScreen.MinimumLoadingScreenDisplayTime = 3.0f;
	LoadingScreen.WidgetLoadingScreen = LoadingWidget->TakeWidget();
	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);

	GetMoviePlayer()->PlayMovie();
	NextLoadingType = ELoadingScreenType::None;
}

void UPTWGameInstance::StopLoadingScreen()
{
	if (GetMoviePlayer()->IsMovieCurrentlyPlaying())
	{
		GetMoviePlayer()->StopMovie();
	}
}

void UPTWGameInstance::LeaveGameSession()
{
	if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
	{
		SteamSessionSubsystem->DestroySession();	
	}
}
