#pragma once
#include "CoreMinimal.h"

namespace GameLift
{
	namespace ClientAPI::Routes
	{
		constexpr const TCHAR* StartMatchmaking		= TEXT("start-matchmaking");
		constexpr const TCHAR* StopMatchmaking		= TEXT("stop-matchmaking");
		constexpr const TCHAR* AcceptMatchmaking	= TEXT("accept-matchmaking");
	
		constexpr const TCHAR* CreatePlayerSession	= TEXT("create-player-session");
		constexpr const TCHAR* SearchGameSessions	= TEXT("search-game-sessions");
	}
}

namespace WebSocket
{
	namespace ClientAPI::Routes
	{
		namespace Send
		{
			constexpr const TCHAR* Ping				= TEXT("ping");
		}
		
		namespace Receive
		{
			constexpr const TCHAR* Pong				= TEXT("Pong");
			constexpr const TCHAR* Matchmaking		= TEXT("Matchmaking");
		}
	}
}