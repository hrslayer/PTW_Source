// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWBaseCharacter.h"
#include "InputActionValue.h"
#include "PTWInputConfig.h"
#include "PTWPlayerState.h"
#include "CoreFramework/PTWPlayerData.h"
#include "PTWPlayerCharacter.generated.h"

class UPTWChaosAffectedComponent;
class USphereComponent;
class APTWPlayerState;
class UPTWItemDefinition;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UPTWInventoryComponent;
class UWidgetComponent;
class UPTWWeaponComponent;
class UPTWReactorComponent;
class UPTWInteractComponent;
class UVOIPTalker;
class USceneCaptureComponent2D;
class UMaterialInterface;

UCLASS()
class PTW_API APTWPlayerCharacter : public APTWBaseCharacter
{
	GENERATED_BODY()
	
public:
	APTWPlayerCharacter();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FORCEINLINE UInputMappingContext* GetDefaultMappingContext() const { return DefaultMappingContext; }
	FORCEINLINE UPTWWeaponComponent* GetWeaponComponent() const { return WeaponComponent; }
	FORCEINLINE UCameraComponent* GetPlayerCamera() const { return PlayerCamera; }
	FORCEINLINE UPTWInventoryComponent* GetInventoryComponent() const
	{
		if (APTWPlayerState* PS = Cast<APTWPlayerState>(GetPlayerState()))
		{
			return PS->GetInventoryComponent();
		}
		return nullptr;
	}
	FORCEINLINE UWidgetComponent* GetNameTagWidget() const { return NameTagWidget; }
	UFUNCTION(BlueprintPure, Category = "Mesh")
	FORCEINLINE USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	UFUNCTION(BlueprintPure, Category = "Mesh")
	FORCEINLINE USkeletalMeshComponent* GetMesh3P() const { return GetMesh(); }
	FORCEINLINE UPTWInteractComponent* GetInteractComponent() const { return InteractComponent; }
	FORCEINLINE bool GetStealthMode() const { return bIsStealth; }
	FORCEINLINE USphereComponent* GetPushCollision() const {return PushCollision; }
	FORCEINLINE UVOIPTalker* GetVOIPTalkerComponent() const { return VOIPTalkerComponent; }
	FORCEINLINE USceneCaptureComponent2D* GetTargetPOVCapture() const { return TargetPOVSource; }
	FORCEINLINE AActor* GetAttacker() const { return LastAttackerActor.Get(); }
	FORCEINLINE UPTWChaosAffectedComponent* GetChaosAffectedComponent() const { return ChaosAffectedComponent; }
	
	void SetStealthMode(bool bSetStealthMode);
	void SetIceVisual(bool bEnable);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetIceVisual(bool bEnable);
	void SetLastAttacker(AActor* Attacker);

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	void EquipFirstWeapon(const FInputActionValue& Value);
	void EquipSecondWeapon(const FInputActionValue& Value);
	void UseActiveItem(const FInputActionValue& Value);

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState) override;
	virtual void InitAbilityActorInfo() override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void HandleDeath(AActor* Attacker) override;
	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;
	
	void InitCharacterState();

	void UpdateNameTagText();
	
	UFUNCTION(Server, Reliable)
	void ServerRPCEquipWeapon(int32 SelectIndex);
	
	UFUNCTION(Server, Reliable)
	void ServerRPCUseActiveItem();

	void RegisterGameplayTagEvents();
	UFUNCTION()
	void OnStasisTagChanged(const FGameplayTag Tag, int32 NewCount);

	UFUNCTION()
	void OnMovelimit(const FGameplayTag Tag, int32 NewCount);

	UFUNCTION()
	void OnRep_StealthMode();

	void StartInitTimer();
	
	void ClearLastAttacker();

public:
	bool bIsAbilitiesInitialized = false;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UPTWInputConfig> InputConfig;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> EquipFirstWeaponAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> EquipSecondWeaponAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> UseActiveItemAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> DropItemAction;
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> LandSound;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Temp")
	TObjectPtr<UPTWItemDefinition> ItemDef;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> NameTagWidget;

	FTimerHandle NameTagRetryTimer;
	FTimerHandle InitTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", ReplicatedUsing = OnRep_StealthMode)
	bool bIsStealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> PlayerCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh1P;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh1PHand;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPTWWeaponComponent> WeaponComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPTWInteractComponent> InteractComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> PushCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UVOIPTalker> VOIPTalkerComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GTS|Capture")
	TObjectPtr<USceneCaptureComponent2D> TargetPOVSource;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ice")
	TObjectPtr<UMaterialInterface> IceOverlayMaterial;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ice")
	TObjectPtr<UPTWChaosAffectedComponent> ChaosAffectedComponent;
	
public:
	UFUNCTION()
	void OnPlayerDataLoaded(const FPTWPlayerData& NewData);

	bool bHasGivenStartupItems = false;
	
private:
	UPROPERTY()
	TWeakObjectPtr<AActor> LastAttackerActor;

	FTimerHandle ClearLastAttackerTimerHandle;
};
