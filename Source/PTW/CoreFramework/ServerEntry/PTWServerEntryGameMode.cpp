#include "PTWServerEntryGameMode.h"
#include "System/PTWSteamSessionSubsystem.h"
#include "System/Server/PTWServerSettings.h"


APTWServerEntryGameMode::APTWServerEntryGameMode()
{
	bUseSeamlessTravel = true;
}

void APTWServerEntryGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	const TSet<UActorComponent*>& Components = GetComponents();
	if (Components.Num() <= 1)
	{
		// GameLift Fleet으로 올리는 서버파일은 리눅스 크로스 컴파일로 올립니다.
		// 그러므로 리눅스 플랫폼이 아닌 경우에는 GameLift SDK를 초기화하지 않습니다.
		if (UPTWSteamSessionSubsystem* SessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
		{
			FPTWServerSettings ServerSettings(
				TEXT("Test 서버"), 
				16, 
				EPTWRoundType::Long, 
				EPTWServerType::Custom, 
				UE_SERVER);
		
			SessionSubsystem->CreateGameSession(ServerSettings);
		}
	}
}