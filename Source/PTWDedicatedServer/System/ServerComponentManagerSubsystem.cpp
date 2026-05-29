#include "ServerComponentManagerSubsystem.h"
#include "Components/GameFrameworkComponentManager.h"
#include "CoreFramework/Game/GameMode/PTWGameMode.h"
#include "CoreFramework/Staging/PTWStagingGameMode.h"
#include "CoreFramework/Game/GameMode/Component/PTWGameLiftConnectionComponent.h"
#include "CoreFramework/Game/GameMode/Component/PTWGameLiftInitializerComponent.h"
#include "CoreFramework/Game/GameMode/Component/PTWGameLiftStagingComponent.h"
#include "CoreFramework/ServerEntry/PTWServerEntryGameMode.h"
#include "Kismet/GameplayStatics.h"


UServerComponentManagerSubsystem::UServerComponentManagerSubsystem()
{
}

void UServerComponentManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UGameFrameworkComponentManager* ComponentManager = GetGameInstance()->GetSubsystem<UGameFrameworkComponentManager>())
	{
		GameModeExtensionHandle = ComponentManager->AddExtensionHandler(
			APTWGameMode::StaticClass(),
			UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleGameModeExtension));
		
		StagingGameModeExtensionHandle = ComponentManager->AddExtensionHandler(
			APTWStagingGameMode::StaticClass(),
			UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleGameModeExtension));
		
		ServerEntryGameModeExtensionHandle = ComponentManager->AddExtensionHandler(
			APTWServerEntryGameMode::StaticClass(),
			UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleGameModeExtension));
	}
}

void UServerComponentManagerSubsystem::Deinitialize()
{
	GameModeExtensionHandle.Reset();
	StagingGameModeExtensionHandle.Reset();
	ServerEntryGameModeExtensionHandle.Reset();
	
	Super::Deinitialize();
}

void UServerComponentManagerSubsystem::HandleGameModeExtension(AActor* Receiver, FName EventName)
{
	if (UWorld* World = Receiver->GetWorld())
	{
		FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(World, true);
		if (CurrentLevelName.Equals(TEXT("TransitionMap"))) return;
	}
	
	if (EventName.IsEqual(UGameFrameworkComponentManager::NAME_ReceiverAdded))
	{
		if (Receiver->IsA<APTWGameMode>())
		{
			AddExtensionComponent<APTWGameMode, UPTWGameLiftConnectionComponent>(Receiver);
			
			if (Receiver->IsA<APTWStagingGameMode>())
			{
				AddExtensionComponent<APTWStagingGameMode, UPTWGameLiftStagingComponent>(Receiver);
			}
		}
		else if (Receiver->IsA<APTWServerEntryGameMode>())
		{
			AddExtensionComponent<APTWServerEntryGameMode, UPTWGameLiftInitializerComponent>(Receiver);
		}
	}
	else if (EventName.IsEqual(UGameFrameworkComponentManager::NAME_ReceiverRemoved))
	{
		if (Receiver->IsA<APTWGameMode>())
		{
			RemoveExtensionComponent<APTWGameMode, UPTWGameLiftConnectionComponent>(Receiver);
			
			if (Receiver->IsA<APTWStagingGameMode>())
			{
				RemoveExtensionComponent<APTWStagingGameMode, UPTWGameLiftStagingComponent>(Receiver);
			}
		}
		else if (Receiver->IsA<APTWServerEntryGameMode>())
		{
			RemoveExtensionComponent<APTWServerEntryGameMode, UPTWGameLiftInitializerComponent>(Receiver);
		}
	}
}
