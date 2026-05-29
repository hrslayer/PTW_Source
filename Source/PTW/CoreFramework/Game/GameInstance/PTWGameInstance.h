#pragma once
#include "CoreMinimal.h"
#include "System/Server/PTWServerSettings.h"
#include "PTWGameInstance.generated.h"

class APTWPlayerState;
class UPTWLoadingWidgetBase;
class USoundMix;
class USoundClass;
struct FComponentRequestHandle;

UENUM(BlueprintType)
enum class ELoadingScreenType : uint8
{
	None,
	Lobby,
	MiniGame
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerUniqueIdSignature, const FString&, UniqueId);


/**
 * PTW GameInstance
 */
UCLASS()
class PTW_API UPTWGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPTWGameInstance(const FObjectInitializer& ObjectInitializer);
	
	/* 트래블 시작 전 호출하여 로딩 데이터를 셋팅 */
	void PrepareLoadingScreen(ELoadingScreenType InType, FName InMapRowName);
	
	/* MoviePlayer 로딩 화면 시작 */
	UFUNCTION()
	virtual void BeginLoadingScreen(const FString& MapName);
	
	/* MoviePlayer 로딩 화면 수동 시작 */
	void DisplayLoadingScreen();

	/* MoviePlayer 로딩 화면 수동 종료 */
	void StopLoadingScreen();

	/** 세션(서버) 접속 종료 로직 */
	UFUNCTION(BlueprintCallable, Category = "Network|Session")
	void LeaveGameSession();
	
protected:
	virtual void Init() override;
	virtual void Shutdown() override;
	
	void HandlePlayerStateExtension(AActor* Receiver, FName EventName);
	TSharedPtr<FComponentRequestHandle> PlayerStateExtensionHandle;
	
public:
	/* 게임 시작 시 사운드 설정 저장값 자동 적용 */
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundMix* MasterSoundMix;
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* MasterSoundClass;
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* BGMSoundClass;
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* SFXSoundClass;
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* UISoundClass;
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* VoiceSoundClass;
	UPROPERTY(EditDefaultsOnly, Category = "Class")
	TSubclassOf<class ACharacter> KeepCharacterClassLoaded;
	UPROPERTY(EditDefaultsOnly, Category = "Class")
	TSubclassOf<class AActor> KeepResultCharacterClassLoaded;
	
	/** 플레이어들의 UniqueId를 보관하는 휘발성 TSet.\n
	 * 레벨이동할때 마다, UniqueId를 추가/제거를 반복합니다. 
	 */
	UPROPERTY(EditDefaultsOnly, Category = "NetWork")
	TSet<FString> LevelPlayerIds;
	
	/** 플레이어들의 UniqueId를 보관하는 비휘발성 TSet.\n
	 * 현재 세션에 접속/종료 할때마다, UniqueId를 추가/제거를 반복합니다.
	 * [메인메뉴로 이동할때 까지 유지]
	 */
	UPROPERTY(EditDefaultsOnly, Category = "NetWork")
	TSet<FString> SessionPlayerIds;
	
	/** 플레이어들의 PlayerName을 보관하는 비휘발성 TMap.\n
	 * Key: UniqueId, Value: PlayerName\n
	 * [메인메뉴로 이동할때 까지 유지]
	 */
	UPROPERTY(EditDefaultsOnly, Category = "NetWork")
	TMap<FString, FString> SessionPlayerNames;
	
	/** 로컬 플레이어의 UniqueId를 저장하는 FString\n
	 * OnlineSubsystem Steam 로그인 후 업데이트.
	 * [절대로 파괴되지 않음]
	 */
	UPROPERTY(BlueprintReadOnly, Category = "NetWork")
	FString LocalUniqueId;
	
	bool bIsFirstLobby = true;
	int32 CurrentPlayerCount = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "NetWork")
	FPTWServerSettings ServerSettings;

protected:
	UPROPERTY()
	ELoadingScreenType NextLoadingType; // 다음이 미니게임인지 로비인지 저장

	UPROPERTY()
	FName TargetMapRowName; // 맵 이름 저장

	/* 로딩 위젯 클래스들(에디터에서 할당) */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPTWLoadingWidgetBase> LobbyLoadingWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPTWLoadingWidgetBase> MiniGameLoadingWidgetClass;

	/* 맵 데이터 테이블 */
	UPROPERTY(EditDefaultsOnly, Category = "Loading")
	UDataTable* MiniGameMapTable;

	/* 로비일 때 사용할 기본 배경 이미지 */
	UPROPERTY(EditDefaultsOnly, Category = "Loading")
	TSoftObjectPtr<UTexture2D> LobbyDefaultImage;
	
public:
	/* 현재 내 UniqueId가 생성(스팀API 로그인)이 완료되면 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Network|Local")
	FOnPlayerUniqueIdSignature OnLocalUniqueIdCreated;
	
	/* 현재 레벨에서 PlayerState에 UniqueId, PlayerName이 세팅되면 호출하는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Network|Level")
	FOnPlayerUniqueIdSignature OnLevelPlayerConnected;
	
	/* 현재 레벨에서 PlayerState가 파괴되면 호출하는 델리게이트 (ex: 클라이언트별 접속종료, 레벨이동) */
	UPROPERTY(BlueprintAssignable, Category = "Network|Level")
	FOnPlayerUniqueIdSignature OnLevelPlayerDisconnected;
	
	/* 현재 세션에서 PlayerState에 UniqueId, PlayerName이 세팅되면 호출하는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Network|Session")
	FOnPlayerUniqueIdSignature OnSessionPlayerConnected;
	
	/* 현재 세션에서 플레이어가 로그아웃하면 호출되는 델리게이트 (접속종료, 크래시) */
	UPROPERTY(BlueprintAssignable, Category = "Network|Session")
	FOnPlayerUniqueIdSignature OnSessionPlayerDisconnected;
};
