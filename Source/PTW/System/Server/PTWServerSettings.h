#pragma once
#include "CoreMinimal.h"
#include "PTWServerSettings.generated.h"

UENUM(BlueprintType)
enum class EPTWServerType : uint8
{
	None		UMETA(DisplayName = "None"),
	Match		UMETA(DisplayName = "Match"),
	Custom		UMETA(DisplayName = "Custom"),
	End			UMETA(DisplayName = "End")
};

UENUM(BlueprintType)
enum class EPTWRoundType : uint8
{
	None		UMETA(DisplayName = "None"),
	Short		UMETA(DisplayName = "Short"),
	Long		UMETA(DisplayName = "Long"),
	End			UMETA(DisplayName = "End")
};

USTRUCT(BlueprintType)
struct FPTWServerSettings
{
	GENERATED_BODY()

	inline static const FName Key_ServerName      = FName(TEXT("ServerName"));
	inline static const FName Key_MaxPlayerCount  = FName(TEXT("MaxPlayerCount"));
	inline static const FName Key_RoundType       = FName(TEXT("RoundType"));
	inline static const FName Key_ServerType      = FName(TEXT("ServerType"));
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FString ServerName = TEXT("");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 MaxPlayerCount {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 MinPlayerCount {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EPTWRoundType RoundType {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EPTWServerType ServerType {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bIsDedicatedServer {};
	
	UPROPERTY()
	bool bIsValid = false;
	
	FPTWServerSettings() = default;
	FPTWServerSettings(const FString& InServerName, const int32 InMaxPlayerCount, const EPTWRoundType InRoundType, 
		const EPTWServerType InServerType, const bool InbIsDedicatedServer) :
		ServerName(InServerName),
		MaxPlayerCount(InMaxPlayerCount),
		MinPlayerCount(1),
		RoundType(InRoundType),
		ServerType(InServerType), 
		bIsDedicatedServer(InbIsDedicatedServer),
		bIsValid(true)
		{}
	FPTWServerSettings(const FString& InServerName, const int32 InMaxPlayerCount, const int32 InMinPlayerCount, const EPTWRoundType InRoundType, 
		const EPTWServerType InServerType, const bool InbIsDedicatedServer) :
		ServerName(InServerName),
		MaxPlayerCount(InMaxPlayerCount),
		MinPlayerCount(InMinPlayerCount),
		RoundType(InRoundType),
		ServerType(InServerType), 
		bIsDedicatedServer(InbIsDedicatedServer), 
		bIsValid(true)
	{}
	
	FORCEINLINE bool IsValid() const { return bIsValid; };
	static int32 RoundTypeToValue(const EPTWRoundType InRoundType);
	static FString RoundTypeToString(const EPTWRoundType InRoundType);
	static EPTWRoundType StringToRoundType(const FString& InString);
	
	static FString ServerTypeToString(const EPTWServerType& ServerType);
	static EPTWServerType StringToServerType(const FString& InString);
};

USTRUCT(BlueprintType)
struct FMatchBackfillData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString TicketId {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString GameSessionId {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString MatchmakingConfiguration {};
	
	UPROPERTY()
	bool bIsValid = false;
	
	FMatchBackfillData() = default;
	FMatchBackfillData(const FString& InTicketId, const FString& InGameSessionId, const FString& InMatchmakingConfiguration) :
		TicketId(InTicketId),
		GameSessionId(InGameSessionId),
		MatchmakingConfiguration(InMatchmakingConfiguration),
		bIsValid(true)
	{}
	
	const bool IsValid();
};

USTRUCT()
struct FPTWGameProperty
{
	GENERATED_BODY()

	UPROPERTY() FString Key{};
	UPROPERTY() FString Value{};
};

USTRUCT(BlueprintType)
struct FPTWGameLiftGameSession
{
	GENERATED_BODY()

	// Json Serialization UPROPERTY
	UPROPERTY() FString CreationTime{};
	UPROPERTY() FString CreatorId{};
	UPROPERTY() int32 CurrentPlayerSessionCount{};
	UPROPERTY() FString DnsName{};
	UPROPERTY() FString FleetArn{};
	UPROPERTY() FString FleetId;
	// UPROPERTY() TMap<FString, FString> GameProperties{};
	UPROPERTY() TArray<FPTWGameProperty> GameProperties{};
	UPROPERTY() FString GameSessionData{};
	UPROPERTY() FString GameSessionId{};
	UPROPERTY() FString IpAddress{};
	UPROPERTY() FString Location{};
	UPROPERTY() FString MatchMakerData{};
	UPROPERTY() int32 MaximumPlayerSessionCount{};
	UPROPERTY() FString Name{};
	UPROPERTY() FString PlayerSessionCreationPolicy{};
	UPROPERTY() int32 Port{};
	UPROPERTY() FString Status{};
	UPROPERTY() FString StatusReason{};
	UPROPERTY() FString TerminationTime{};
	
	void Dump() const;
};

USTRUCT(BlueprintType)
struct FPTWGameLiftPlayerSession
{
	GENERATED_BODY()

	// Json Serialization UPROPERTY
	UPROPERTY() FString CreationTime{};
	UPROPERTY() FString DnsName{};
	UPROPERTY() FString FleetArn{};
	UPROPERTY() FString FleetId;
	UPROPERTY() FString GameSessionId{};
	UPROPERTY() FString IpAddress{};
	UPROPERTY() FString PlayerData{};
	UPROPERTY() FString PlayerId{};
	UPROPERTY() FString PlayerSessionId{};
	UPROPERTY() int32 Port{};
	UPROPERTY() FString Status{};
	UPROPERTY() FString TerminationTime{};
	
	void Dump() const;
};

USTRUCT(BlueprintType)
struct FPTWGameSessionListsTable
{
	GENERATED_BODY()

	UPROPERTY() FString GameSessionId;
	UPROPERTY() FString SteamId;
	UPROPERTY() FString ServerState;
	UPROPERTY() FString ServerName;
	UPROPERTY() int32 MaxPlayers;
	UPROPERTY() int32 CurrentPlayerCount;
	UPROPERTY() FString MaxRoundType;
	UPROPERTY() int64 CreatedAt;
	UPROPERTY() int64 DeleteAt;
};
