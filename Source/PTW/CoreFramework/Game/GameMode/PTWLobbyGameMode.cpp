// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWLobbyGameMode.h"

#include "PTWLobbyItemManager.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "Debug/PTWLogCategorys.h"
#include "MiniGame/Manager/PTWRouletteEventManager.h"
#include "PTW/CoreFramework/Game/GameState/PTWGameState.h"
#include "System/PTWScoreSubsystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Actor/PTWResultCharacter.h"

APTWLobbyGameMode::APTWLobbyGameMode()
{
	//LobbyItemManager = CreateDefaultSubobject<UPTWLobbyItemManager>(TEXT("LobbyItemManager"));
	RouletteEventManager = CreateDefaultSubobject<UPTWRouletteEventManager>(TEXT("RouletteEventManager"));
	
	bStartPlayersAsSpectators = false;
}

void APTWLobbyGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	// 이 부분은 매칭 방식이 바뀌면 필요 없음 
	// if (UPTWGameInstance* PTWGameInstance = Cast<UPTWGameInstance>(GetGameInstance()))
	// {
	// 	if (PTWGameInstance->bIsFirstLobby == true && bSkipFirstLobby == false)
	// 	{
	// 		bIsFirstLobby = true;	
	// 		PTWGameInstance->bIsFirstLobby = false;
	// 	}
	// 	else
	// 	{
	// 		bIsFirstLobby = false;
	// 		PTWGameInstance->bIsFirstLobby = false;
	// 	}
	// }
	
	if (UPTWGameInstance* GI = GetGameInstance<UPTWGameInstance>())
	{
		const FPTWServerSettings& ServerSetting = GI->ServerSettings;
		if (ServerSetting.IsValid())
		{
			GameFlowRule.MaxPlayers = ServerSetting.MaxPlayerCount;
			GameFlowRule.MaxRound = FPTWServerSettings::RoundTypeToValue(ServerSetting.RoundType);
		}
		else
		{
			GameFlowRule.MaxPlayers = 16;
			GameFlowRule.MaxRound = 5;
		}
	}

	if (!ScoreSubsystem) return;
	ScoreSubsystem->ResetRoleData();
}

void APTWLobbyGameMode::InitGameState()
{
	Super::InitGameState();
	// 이 부분은 매칭 방식이 바뀌면 필요 없음 
	if (IsValid(PTWGameState))
	{
		PTWGameState->SetCurrentPhase(EPTWGamePhase::Loading);
		TravelLevelName = TEXT("/Game/_PTW/Maps/MiniGame_Bomb");
	}
}

void APTWLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (RouletteEventManager)
	{
		RouletteEventManager->OnRouletteFinished.AddDynamic(this, &APTWLobbyGameMode::OnRouletteFinished);
	}

	if (!PTWGameState) return;
	
	LobbyItemManager = NewObject<UPTWLobbyItemManager>(this);
	LobbyItemManager->InitLobbyItemManager(LobbyItemDataTable, PTWGameState);
	
}

void APTWLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (!IsValid(PTWGameState)) return;
	
	// 매칭 방식 바뀌면 추가
	
	APTWPlayerState* PlayerState = NewPlayer->GetPlayerState<APTWPlayerState>();
	if (!PlayerState) return;
	
	if (bIsGameStarted)
	{
		PlayerState->bWaitingForNextGame = true;
		
		NewPlayer->UnPossess();
		NewPlayer->PlayerState->SetIsSpectator(true);
		NewPlayer->ChangeState(NAME_Spectating);
		NewPlayer->ClientGotoState(NAME_Spectating);
	}
}

void APTWLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	// 로그 아웃하면 gamestate portal 부분 수정
}

void APTWLobbyGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	UE_LOG(Log_LobbyGameMode, Warning, TEXT("[로비게임모드] HandleSeamlessTravelPlayer: %s"), *C->GetName());
	
	Super::HandleSeamlessTravelPlayer(C);
	
	PlayerReadyToPlay(Cast<APTWPlayerController>(C));
}
void APTWLobbyGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	APTWPlayerState* PTWPlayerState = NewPlayer->GetPlayerState<APTWPlayerState>();
	if (!PTWPlayerState) return;

	if (!LobbyItemManager) return;
	
	// 데이터 초기화 및 골드 지급
	PTWPlayerState->ResetInventoryItemId();
}

void APTWLobbyGameMode::PlayerReadyToPlay(APlayerController* Controller)
{
	Super::PlayerReadyToPlay(Controller);

	UE_LOG(Log_LobbyGameMode, Warning, TEXT("[Lobby] PlayerReadyToPlay: %s, %d/%d"), *Controller->GetName(),ReadyPlayer, AllPlayer);
	
	if (!IsValid(PTWGameState) || !Controller) return;
	
	APTWPlayerState* PTWPlayerState = Controller->GetPlayerState<APTWPlayerState>();
	if (!PTWPlayerState) return;
	
	PTWPlayerState->bIsReadyToPlay = true;
	
	if (ReadyPlayer >= AllPlayer)
	{
		// 나중에 이부분 함수화 예정
		if (bAllPlayerReady) return;
		bAllPlayerReady = true;
		
		GetWorldTimerManager().ClearTimer(LoadingDelayTimer);
		GetWorldTimerManager().SetTimer(LoadingDelayTimer, this, &APTWLobbyGameMode::StartGameLobby, 3.f);
	}
}


void APTWLobbyGameMode::StartGameLobby()
{
	if (bIsGameStarted) return;
	bIsGameStarted = true;
	if (!IsValid(PTWGameState) || !IsValid(ScoreSubsystem)) return;
	
	GetWorldTimerManager().ClearTimer(TestTimer);
	ClearTimer();
	
	PTWGameState->InitLobbyRankingDataMap(ScoreSubsystem->GetKnownPlayersGameData());
	GiveLobbyGold();
	
	// 최대 라운드에 도달 하면 게임 종료
	if (PTWGameState->GetCurrentRound() >= GameFlowRule.MaxRound)
	{
		EndGame();
		
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Start Game Lobby"));
	
	PTWGameState->AdvanceRound();
	
	
	PTWGameState->SetCurrentPhase(EPTWGamePhase::Lobby);

	for (APlayerState* PS : PTWGameState->PlayerArray)
	{
		AController* PC = PS->GetPlayerController();
		if (!PC) continue;
		
		//SetInputBlock(false);
	}
	
	if (!GetWorldTimerManager().IsTimerActive(TimerHandle))
	{
		StartTimer(GameFlowRule.NextMiniGameWaitTime);
	
		StartRoulette();
	}
	
}

void APTWLobbyGameMode::EndTimer()
{
	if (!PTWGameState || !RouletteEventManager) return;

	const FName SelectedMapName = RouletteEventManager->GetMapRowName(TravelLevelName);
	PrepareAllPlayersLoadingScreen(ELoadingScreenType::MiniGame, SelectedMapName);
	PTWGameState->AddPlayedMap(SelectedMapName);
	
	PTWGameState->SetCurrentMap(*RouletteEventManager->GetMiniGameMapRow(SelectedMapName));
	Super::EndTimer();
}

bool APTWLobbyGameMode::IsWinner(APTWPlayerState* InPlayerState)
{
	if (!PTWGameState) return false;

	PTWGameState->SortLobbyRankingData();

	const FUniqueNetIdRepl& PlayerId = InPlayerState->GetUniqueId();

	const FPTWLobbyRankingData* Data = PTWGameState->GetLobbyGameRankingData().Find(PlayerId.ToString());
	if (!Data) return false;

	return Data->Rank == 1;
}

void APTWLobbyGameMode::ExitSpectorMode(AController* Controller)
{
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!PC) return;

	// 관전 상태일 때만 처리
	if (!PC->PlayerState || !PC->PlayerState->IsSpectator()) return;

	PC->ChangeState(NAME_Playing);
	PC->ClientGotoState(NAME_Playing);

	PC->PlayerState->SetIsSpectator(false);
	PC->PlayerState->SetIsOnlyASpectator(false);
}

void APTWLobbyGameMode::StartRoulette()
{
	//SelectedRandomMap();
	//SelectedRandomEvent();

	if (!RouletteEventManager) return;
	
	RouletteEventManager->StartRouletteSequence();
}


void APTWLobbyGameMode::OnRouletteFinished(FName SelectedMapName)
{
	if (!RouletteEventManager) return;
	
	TravelLevelName = RouletteEventManager->TravelLevelName;
}

void APTWLobbyGameMode::EndGame()
{
	StartResultSequence();
}

void APTWLobbyGameMode::GiveLobbyGold()
{
	if (!ScoreSubsystem || !PTWGameState) return;

	ScoreSubsystem->AddLobbyGoldToAllPlayers(RoundClearBonusGold);
	
	for (APlayerState* PlayerState : PTWGameState->PlayerArray)
	{
		if (APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState))
		{
			const FString& PlayerId = PTWPlayerState->GetUniqueId().ToString();
			if (PlayerId.IsEmpty()) continue;

			if (const FPTWPlayerGameData* Data = ScoreSubsystem->FindPlayerGameData(PlayerId))
			{
				PTWPlayerState->SetPlayerData(Data->PlayerData);
			}
		}
	}
}

void APTWLobbyGameMode::ApplyLobbyItem(APTWPlayerState* Buyer, const FName ItemId)
{
	if (!LobbyItemManager) return;
	
	LobbyItemManager->ApplyLobbyItem(Buyer, ItemId);
}

void APTWLobbyGameMode::AddChaosItemEntry(const FPTWChaosItemEntry& Entry)
{
	if (!PTWGameState) return;

	PTWGameState->AddChaosItemEntry(Entry);
}

void APTWLobbyGameMode::AddGold(APTWPlayerState* PlayerState, int32 Amount)
{
	FPTWPlayerData PlayerData = PlayerState->GetPlayerData();
	PlayerData.Gold += Amount; 
	PlayerState->SetPlayerData(PlayerData);

	if (!PTWGameState) return;

	PTWGameState->UpdateLobbyRankingDataMap(PlayerState->GetUniqueId().ToString(), PlayerState->GetPlayerData());
}


