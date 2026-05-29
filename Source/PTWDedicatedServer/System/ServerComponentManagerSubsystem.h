#pragma once
#include "CoreMinimal.h"
#include "Debug/PTWLogCategorys.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ServerComponentManagerSubsystem.generated.h"


struct FComponentRequestHandle;

/**
 * 서버 전용 컴포넌트들을 관리하는 서브시스템입니다.
 */
UCLASS()
class PTWDEDICATEDSERVER_API UServerComponentManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UServerComponentManagerSubsystem();
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	template <typename TActor, typename TComponent>
	bool AddExtensionComponent(AActor* Receiver);
	
	template <typename TActor, typename TComponent>
	bool RemoveExtensionComponent(AActor* Receiver);
	
	void HandleGameModeExtension(AActor* Receiver, FName EventName);
	
protected:
	TSharedPtr<FComponentRequestHandle> GameModeExtensionHandle;
	TSharedPtr<FComponentRequestHandle> StagingGameModeExtensionHandle;
	TSharedPtr<FComponentRequestHandle> ServerEntryGameModeExtensionHandle;
	
};

template <typename TActor, typename TComponent>
bool UServerComponentManagerSubsystem::AddExtensionComponent(AActor* Receiver)
{
	FString ComponentName = TComponent::StaticClass()->GetName();
	FString ActorName = TActor::StaticClass()->GetName();
		
	if (Receiver->IsA<TActor>())
	{
		if (!Receiver->FindComponentByClass<TComponent>())
		{
			TComponent* Component = NewObject<TComponent>(Receiver);
			Receiver->AddInstanceComponent(Component);
			Component->RegisterComponent();

			UE_LOG(Log_Server, Log, TEXT("%s가 %s에 성공적으로 주입되었습니다."), *ComponentName, *ActorName);
			return true;
		}
		else
		{
			UE_LOG(Log_Server, Warning, TEXT("%s가 %s에 이미 주입되있습니다."), *ComponentName, *ActorName);
			return false;
		}
	}
	else
	{
		UE_LOG(Log_Server, Error, TEXT("%s가 %s에 주입에 실패하였습니다."), *ComponentName, *ActorName);
		return false;
	}
}

template <typename TActor, typename TComponent>
bool UServerComponentManagerSubsystem::RemoveExtensionComponent(AActor* Receiver)
{
	FString ComponentName = TComponent::StaticClass()->GetName();
	FString ActorName = TActor::StaticClass()->GetName();
		
	if (Receiver->IsA<TActor>())
	{
		if (TComponent* Component = Receiver->FindComponentByClass<TComponent>())
		{
			Receiver->RemoveInstanceComponent(Component);
			Component->DestroyComponent();
			
			UE_LOG(Log_Server, Log, TEXT("%s가 %s에 성공적으로 해제 되었습니다."), *ComponentName, *ActorName);
			return true;
		}
		else
		{
			UE_LOG(Log_Server, Warning, TEXT("%s가 %s에 이미 해제 되었습니다."), *ComponentName, *ActorName);
			return false;
		}
	}
	else
	{
		UE_LOG(Log_Server, Error, TEXT("%s가 %s에 해제에 실패하였습니다."), *ComponentName, *ActorName);
		return false;
	}
}