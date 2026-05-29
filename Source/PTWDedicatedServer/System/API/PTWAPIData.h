#pragma once
#include "CoreMinimal.h"


namespace GameLift
{
	namespace ServerAPI::Routes
	{
		constexpr const TCHAR* ActivateGameSession	= TEXT("activate-game-session");
		constexpr const TCHAR* HeartbeatGameSession	= TEXT("heartbeat-game-session");
		constexpr const TCHAR* TerminateGameSession	= TEXT("terminate-game-session");
		constexpr const TCHAR* UpdatePlayerCount	= TEXT("update-player-count");
	}
}