#pragma once
#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "PTWServerEntryGameMode.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInitializeGameLiftServer);

/**
 * 
 */
UCLASS()
class PTW_API APTWServerEntryGameMode : public AModularGameMode
{
	GENERATED_BODY()

public:
	APTWServerEntryGameMode();
	
protected:
	virtual void BeginPlay() override;
	
public:
	FOnInitializeGameLiftServer OnInitializeGameLiftServer;
};
