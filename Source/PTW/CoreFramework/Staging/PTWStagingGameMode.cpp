#include "PTWStagingGameMode.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"


APTWStagingGameMode::APTWStagingGameMode()
{
	TravelLevelName = TEXT("/Game/_PTW/Maps/Lobby");
}

void APTWStagingGameMode::BeginPlay()
{
	Super::BeginPlay();
	
}

void APTWStagingGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void APTWStagingGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (!GetWorldTimerManager().IsTimerActive(TimerHandle))
	{
		StartTimer(WaitingTime);
	}
}

void APTWStagingGameMode::UpdateTimer()
{
	Super::UpdateTimer();
	
	if (IsValid(PTWGameState))
	{
		OnAutoBackfillTimeoutRemainTime.Broadcast(PTWGameState->GetRemainTime());
	}
}