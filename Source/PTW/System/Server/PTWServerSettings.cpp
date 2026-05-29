#include "PTWServerSettings.h"

int32 FPTWServerSettings::RoundTypeToValue(const EPTWRoundType RoundType)
{
	switch (RoundType)
	{
	case EPTWRoundType::Short:
		return 5;
	case EPTWRoundType::Long:
		return 10;
	default:
		return 0;
	}
}

FString FPTWServerSettings::RoundTypeToString(const EPTWRoundType RoundType)
{
	switch (RoundType)
	{
	case EPTWRoundType::Short:
		return TEXT("Short");
	case EPTWRoundType::Long:
		return TEXT("Long");
	default:
		return TEXT("");
	}
}

EPTWRoundType FPTWServerSettings::StringToRoundType(const FString& InString)
{
	if (InString == RoundTypeToString(EPTWRoundType::Short)) return EPTWRoundType::Short;
	if (InString == RoundTypeToString(EPTWRoundType::Long)) return EPTWRoundType::Long;
	
	return EPTWRoundType::None;
}

FString FPTWServerSettings::ServerTypeToString(const EPTWServerType& InServerType)
{
	switch (InServerType)
	{
	case EPTWServerType::Match:
		return TEXT("Match");
	case EPTWServerType::Custom:
		return TEXT("Custom");
	default:
		return TEXT("");
	}
}

EPTWServerType FPTWServerSettings::StringToServerType(const FString& InString)
{
	if (InString == ServerTypeToString(EPTWServerType::Match)) return EPTWServerType::Match;
	if (InString == ServerTypeToString(EPTWServerType::Custom)) return EPTWServerType::Custom;
	
	return EPTWServerType::None;
}

const bool FMatchBackfillData::IsValid()
{
	if (!bIsValid)
	{
		if (TicketId.IsEmpty() || GameSessionId.IsEmpty() || MatchmakingConfiguration.IsEmpty()) return false;
		
		bIsValid = true;
	}
	
	return true;
}

void FPTWGameLiftGameSession::Dump() const
{
	UE_LOG(LogTemp, Log, TEXT("=== Game Session Info ==="));
	UE_LOG(LogTemp, Log, TEXT("CreationTime: %s"), *CreationTime);
	UE_LOG(LogTemp, Log, TEXT("CreatorId: %s"), *CreatorId);
	UE_LOG(LogTemp, Log, TEXT("CurrentPlayerSessionCount: %d"), CurrentPlayerSessionCount);
	UE_LOG(LogTemp, Log, TEXT("DnsName: %s"), *DnsName);
	UE_LOG(LogTemp, Log, TEXT("FleetArn: %s"), *FleetArn);
	UE_LOG(LogTemp, Log, TEXT("FleetId: %s"), *FleetId);
	
	UE_LOG(LogTemp, Log, TEXT("GameProperties: "));
	for (const auto& Pair : GameProperties)
	{
		UE_LOG(LogTemp, Log, TEXT("	%s: %s"), *Pair.Key, *Pair.Value);
	}
	
	UE_LOG(LogTemp, Log, TEXT("GameSessionData: %s"), *GameSessionData);
	UE_LOG(LogTemp, Log, TEXT("GameSessionId: %s"), *GameSessionId);
	UE_LOG(LogTemp, Log, TEXT("IPAddress: %s"), *IpAddress);
	UE_LOG(LogTemp, Log, TEXT("Location: %s"), *Location);
	UE_LOG(LogTemp, Log, TEXT("MatchMakerData: %s"), *MatchMakerData);
	UE_LOG(LogTemp, Log, TEXT("MaximumPlayerSessionCount: %d"), MaximumPlayerSessionCount);
	UE_LOG(LogTemp, Log, TEXT("Name: %s"), *Name);
	UE_LOG(LogTemp, Log, TEXT("PlayerSessionCreationPolicy: %s"), *PlayerSessionCreationPolicy);
	UE_LOG(LogTemp, Log, TEXT("Port: %d"), Port);
	UE_LOG(LogTemp, Log, TEXT("Status: %s"), *Status);
	UE_LOG(LogTemp, Log, TEXT("StatusReason: %s"), *StatusReason);
	UE_LOG(LogTemp, Log, TEXT("TerminationTime: %s"), *TerminationTime);
	UE_LOG(LogTemp, Log, TEXT("========================="));
	
}

void FPTWGameLiftPlayerSession::Dump() const
{
	UE_LOG(LogTemp, Log, TEXT("=== Player Session Info ==="));
	UE_LOG(LogTemp, Log, TEXT("CreationTime: %s"), *CreationTime);
	UE_LOG(LogTemp, Log, TEXT("DnsName: %s"), *DnsName);
	UE_LOG(LogTemp, Log, TEXT("FleetArn: %s"), *FleetArn);
	UE_LOG(LogTemp, Log, TEXT("FleetId: %s"), *FleetId);
	UE_LOG(LogTemp, Log, TEXT("GameSessionId: %s"), *GameSessionId);
	UE_LOG(LogTemp, Log, TEXT("IPAddress: %s"), *IpAddress);
	UE_LOG(LogTemp, Log, TEXT("PlayerData: %s"), *PlayerData);
	UE_LOG(LogTemp, Log, TEXT("PlayerId: %s"), *PlayerId);
	UE_LOG(LogTemp, Log, TEXT("PlayerSessionId: %s"), *PlayerSessionId);
	UE_LOG(LogTemp, Log, TEXT("Port: %d"), Port);
	UE_LOG(LogTemp, Log, TEXT("Status: %s"), *Status);
	UE_LOG(LogTemp, Log, TEXT("TerminationTime: %s"), *TerminationTime);
	UE_LOG(LogTemp, Log, TEXT("========================="));
}
