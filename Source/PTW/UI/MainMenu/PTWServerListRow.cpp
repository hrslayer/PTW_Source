#include "PTWServerListRow.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "PTW/System/PTWSteamSessionSubsystem.h"
#include "System/Server/PTWServerSettings.h"


void UPTWServerListRow::SetupSessionMinimalInfo(const FOnlineSessionSearchResultBP& SearchResult)
{
	SteamSessionInfo = SearchResult.OnlineSessionSearchResult;
	const FOnlineSession& OnlineSession = SteamSessionInfo.Session;
	
	// 위젯 설정
	// 서버 이름 설정
	FString ServerNameString;
	if (OnlineSession.SessionSettings.Get(FPTWServerSettings::Key_ServerName, ServerNameString))
	{
		ServerName->SetText(FText::FromString(ServerNameString));
	}
	
	// 라운드 유형 설정
	FString RoundTypeString;
	if (OnlineSession.SessionSettings.Get(FPTWServerSettings::Key_RoundType, RoundTypeString))
	{
		RoundLimitType->SetText(FText::FromString(RoundTypeString));
	}
	
	// 플레이어 수 설정
	int32 OpenSlots = OnlineSession.NumOpenPublicConnections;
	int32 MaxPlayers = OnlineSession.SessionSettings.NumPublicConnections;
	
	const FString PlayerCount = FString::Printf(TEXT("%d / %d"), 
		MaxPlayers - OpenSlots, MaxPlayers);
	ServerPlayers->SetText(FText::FromString(PlayerCount));
	
	// 핑 설정
	const FString ServerPingStr = FString::Printf((TEXT("%dms")), SteamSessionInfo.PingInMs);
	ServerPing->SetText(FText::FromString(ServerPingStr));
}

void UPTWServerListRow::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (IsValid(JoinButton))
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedJoinButton);
	}
}

void UPTWServerListRow::NativeDestruct()
{
	if (IsValid(JoinButton))
	{
		JoinButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClickedJoinButton);
	}
	
	Super::NativeDestruct();
}

void UPTWServerListRow::OnClickedJoinButton()
{
	FOnlineSessionSearchResultBP SteamSessionInfoBP = FOnlineSessionSearchResultBP(SteamSessionInfo);

	// 리슨 서버 접속
	if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
	{
		SteamSessionSubsystem->JoinGameSession(SteamSessionInfoBP);
	}
}
