// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTagContainer.h"
#include "Camera/CameraComponent.h"
#include "AbilitySystemComponent.h"
#include "PTW.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SceneCaptureComponent2D.h"

#include "PTWPlayerState.h"
#include "PTWInputComponent.h"
#include "PTWPlayerController.h"
#include "PTWGameUserSettings.h"
#include "Character/Component/PTWChaosAffectedComponent.h"
#include "Components/SphereComponent.h"
#include "GAS/PTWGameplayAbility.h"
#include "System/PTWItemSpawnManager.h"
#include "PTW/GAS/PTWAbilitySystemComponent.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWItemDefinition.h"
#include "UI/CharacterUI/PTWPlayerName.h"
#include "CoreFramework/Character/Component/PTWWeaponComponent.h"
#include "CoreFramework/Character/Component/PTWReactorComponent.h"
#include "CoreFramework/Character/Component/PTWInteractComponent.h"
#include "CoreFramework/MainMenu/PTWMainMenuPlayerController.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "MiniGame/PTWMiniGameMode.h"
#include "Net/VoiceConfig.h"

APTWPlayerCharacter::APTWPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationYaw = true;

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(RootComponent);
	PlayerCamera->bUsePawnControlRotation = true;

	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetupAttachment(PlayerCamera);
	Mesh1P->SetHiddenInGame(true);
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetCastShadow(false);
	Mesh1PHand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1PHand"));
	Mesh1PHand->SetupAttachment(Mesh1P);
	Mesh1PHand->SetHiddenInGame(true);
	Mesh1PHand->SetLeaderPoseComponent(Mesh1P);
	Mesh1PHand->SetOnlyOwnerSee(true);
	Mesh1PHand->SetCastShadow(false);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	CrouchedEyeHeight = 40.0f;

	NameTagWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameTagWidget"));
	NameTagWidget->SetupAttachment(GetMesh());
	NameTagWidget->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	NameTagWidget->SetWidgetSpace(EWidgetSpace::Screen);
	NameTagWidget->SetDrawAtDesiredSize(true);

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	WeaponComponent = CreateDefaultSubobject<UPTWWeaponComponent>(TEXT("WeaponComponent"));
	WeaponComponent->SetIsReplicated(true);

	InteractComponent = CreateDefaultSubobject<UPTWInteractComponent>(TEXT("InteractComponent"));
	InteractComponent->SetIsReplicated(true);
	
	PushCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	PushCollision->SetupAttachment(GetMesh(), TEXT("Hand_R_Socket")); 
	PushCollision->SetSphereRadius(30.0f);
	
	PushCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PushCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	PushCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	VOIPTalkerComponent = CreateDefaultSubobject<UVOIPTalker>(TEXT("VOIPTalker"));

	TargetPOVSource = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("TargetPOVSource"));
	if (PlayerCamera)
	{
		TargetPOVSource->SetupAttachment(PlayerCamera);
	}
	TargetPOVSource->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	TargetPOVSource->bCaptureEveryFrame = false;
	TargetPOVSource->bCaptureOnMovement = false;
	TargetPOVSource->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

	ChaosAffectedComponent = CreateDefaultSubobject<UPTWChaosAffectedComponent>(TEXT("ChaosAffectedComponent"));
}

void APTWPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, bIsStealth);
}

void APTWPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
	if (Mesh1P)
	{
		Mesh1P->SetHiddenInGame(false);
		Mesh1P->SetVisibility(true);
	}
	if (Mesh1PHand)
	{
		Mesh1PHand->SetHiddenInGame(false);
		Mesh1PHand->SetVisibility(true);
	}
}

void APTWPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (DefaultMappingContext)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
		{
			if (MoveAction)
			{
				EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APTWPlayerCharacter::Move);
			}
			if (LookAction)
			{
				EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APTWPlayerCharacter::Look);
			}
			if (EquipFirstWeaponAction && EquipSecondWeaponAction)
			{
				EnhancedInputComponent->BindAction(EquipFirstWeaponAction, ETriggerEvent::Started, this, &APTWPlayerCharacter::EquipFirstWeapon);
				EnhancedInputComponent->BindAction(EquipSecondWeaponAction, ETriggerEvent::Started, this, &APTWPlayerCharacter::EquipSecondWeapon);
			}
			if (UseActiveItemAction)
			{
				EnhancedInputComponent->BindAction(UseActiveItemAction, ETriggerEvent::Started, this, &APTWPlayerCharacter::UseActiveItem);
			}

			if (ReactorComponent)
			{
				if (MoveAction)
				{
					EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, ReactorComponent.Get(), &UPTWReactorComponent::OnInputTriggered);
					EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, ReactorComponent.Get(), &UPTWReactorComponent::OnInputCompleted);
				}
				if (LookAction)
				{
					EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, ReactorComponent.Get(), &UPTWReactorComponent::OnInputTriggered);
					EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, ReactorComponent.Get(), &UPTWReactorComponent::OnInputCompleted);
				}
			}

			UPTWInputComponent* PTWInputComp = CastChecked<UPTWInputComponent>(PlayerInputComponent);

			TArray<uint32> BindHandles;
			PTWInputComp->BindAbilityActions(
				InputConfig,
				this,
				&ThisClass::Input_AbilityInputTagPressed,
				&ThisClass::Input_AbilityInputTagReleased,
				BindHandles
			);
		}
	}
}

void APTWPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorldTimerManager().IsTimerActive(NameTagRetryTimer))
	{
		GetWorldTimerManager().ClearTimer(NameTagRetryTimer);
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ClearActorInfo();
	}

	if (APTWPlayerState* PS = GetPlayerState<APTWPlayerState>())
	{
		PS->OnPlayerDataUpdated.RemoveDynamic(this, &APTWPlayerCharacter::OnPlayerDataLoaded);
	}

	Super::EndPlay(EndPlayReason);
}

void APTWPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitCharacterState();
	RegisterGameplayTagEvents();

	if (ChaosAffectedComponent)
	{
		ChaosAffectedComponent->ApplyCurrentChaosEvent();
	}
}

void APTWPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitCharacterState();
	RegisterGameplayTagEvents();
}

void APTWPlayerCharacter::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);

	if (NewPlayerState)
	{
		InitCharacterState();
	}
}

void APTWPlayerCharacter::InitAbilityActorInfo()
{
	Super::InitAbilityActorInfo();

	APTWPlayerState* PS = GetPlayerState<APTWPlayerState>();
	if (PS)
	{
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);
		AbilitySystemComponent = PS->GetAbilitySystemComponent();
		AbilitySystemComponent->RefreshAbilityActorInfo();
		AttributeSet = PS->GetAttributeSet();
	}
}

void APTWPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (LandSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LandSound, GetActorLocation());
	}
}

void APTWPlayerCharacter::HandleDeath(AActor* Attacker)
{
	if (!HasAuthority()) return;
	if (!AbilitySystemComponent) return;

	Super::HandleDeath(Attacker);

	if (APTWPlayerController* PTWPC = GetController<APTWPlayerController>())
	{
		PTWPC->StartSpectating();
	}
}

void APTWPlayerCharacter::CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult)
{
	if (!IsLocallyControlled())
	{
		FRotator CurrentRotation = PlayerCamera->GetRelativeRotation();
		FRotator TargetRotation = CurrentRotation;
		TargetRotation.Pitch = GetBaseAimRotation().Pitch;
		
		FRotator NewInterpRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 15.0f);
		PlayerCamera->SetRelativeRotation(NewInterpRotation);
	}
	
	Super::CalcCamera(DeltaTime, OutResult);
}

void APTWPlayerCharacter::SetLastAttacker(AActor* Attacker)
{
	if (Attacker == this || Attacker == nullptr) return;
	
	LastAttackerActor = Attacker;
	
	GetWorld()->GetTimerManager().SetTimer(
		ClearLastAttackerTimerHandle,
		this,
		&APTWPlayerCharacter::ClearLastAttacker,
		6.0f,
		false
	);
}

void APTWPlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(Controller))
		{
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			if (ChaosAffectedComponent)
			{
				AddMovementInput(ChaosAffectedComponent->bInputInvert ? -ForwardDirection : ForwardDirection, MovementVector.Y);
				AddMovementInput(ChaosAffectedComponent->bInputInvert ? -RightDirection : RightDirection, MovementVector.X);
			}
			else
			{
				AddMovementInput(ForwardDirection, MovementVector.Y);
				AddMovementInput(RightDirection, MovementVector.X);
			}
			
		}
	}
}

void APTWPlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	bool bInvertYSettings = false;
	if (GEngine)
	{
		if (UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings()))
		{
			bInvertYSettings = Settings->bInvertYAxis;
		}
	}

	if (APTWPlayerController* PC = Cast<APTWPlayerController>(GetController()))
	{
		float InvertMultiplierX = 1.f;
		float InvertMultiplierY = 1.f;
		
		if (ChaosAffectedComponent)
		{
			if (ChaosAffectedComponent->bLookActionFrozen) return;

			InvertMultiplierX = ChaosAffectedComponent->bInputInvert ? -1.f : 1.f;
			InvertMultiplierY = ChaosAffectedComponent->bInputInvert ? -1.f : 1.f;
		}
		
		if (bInvertYSettings)
		{
			InvertMultiplierY *= -1.f;
		}

		float FinalYaw = LookAxisVector.X * PC->CurrentMouseSensitivity * InvertMultiplierX;
		float FinalPitch = LookAxisVector.Y * PC->CurrentMouseSensitivity * InvertMultiplierY;

		AddControllerYawInput(FinalYaw);
		AddControllerPitchInput(FinalPitch);
	}
	else if (APTWMainMenuPlayerController* MPC = Cast<APTWMainMenuPlayerController>(GetController()))
	{
		float FinalYaw = LookAxisVector.X * MPC->CurrentMouseSensitivity;
		float FinalPitch = LookAxisVector.Y * MPC->CurrentMouseSensitivity;

		if (bInvertYSettings)
		{
			FinalPitch *= -1.f;
		}

		AddControllerYawInput(FinalYaw);
		AddControllerPitchInput(FinalPitch);
	}
}

void APTWPlayerCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	UE_LOG(LogTemp, Warning, TEXT("[Input_Test] 좌클릭 입력 수신됨! 전달받은 태그: %s"), *InputTag.ToString());

	if (UPTWAbilitySystemComponent* PTWASC = Cast<UPTWAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		PTWASC->AbilityInputTagPressed(InputTag);
		UE_LOG(LogTemp, Warning, TEXT("[Input_Test] ASC에게 어빌리티 발동 요청 시도..."));
	}
}

void APTWPlayerCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (UPTWAbilitySystemComponent* PTWASC = Cast<UPTWAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		PTWASC->AbilityInputTagReleased(InputTag);
	}
}

void APTWPlayerCharacter::InitCharacterState()
{
	APTWPlayerState* PS = GetPlayerState<APTWPlayerState>();

	if (!PS)
	{
		StartInitTimer();
		return;
	}

	if (bIsAbilitiesInitialized)
	{
		GetWorldTimerManager().ClearTimer(InitTimerHandle);
		return;
	}

	InitAbilityActorInfo();

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		StartInitTimer();
		return;
	}

	if (HasAuthority())
	{
		if (ASC->HasMatchingGameplayTag(GameplayTags::State::Status_Dead))
		{
			ASC->RemoveLooseGameplayTag(GameplayTags::State::Status_Dead);
		}

		FGameplayTag EquipTag = FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip"));
		if (ASC->HasMatchingGameplayTag(EquipTag))
		{
			ASC->SetLooseGameplayTagCount(EquipTag, 0);
			ASC->RemoveActiveEffectsWithTags(FGameplayTagContainer(EquipTag));
		}

		if (!bHasGivenStartupItems)
		{
			FPTWPlayerData CurrentData = PS->GetPlayerData();
			if (CurrentData.InventoryItemIDs.Num() > 0)
			{

			}
			else
			{
				PS->OnPlayerDataUpdated.RemoveDynamic(this, &APTWPlayerCharacter::OnPlayerDataLoaded);
				PS->OnPlayerDataUpdated.AddDynamic(this, &APTWPlayerCharacter::OnPlayerDataLoaded);
			}
		}

		GiveDefaultAbilities();
		ApplyDefaultEffects();
	}

	UpdateNameTagText();

	if (VOIPTalkerComponent)
	{
		VOIPTalkerComponent->RegisterWithPlayerState(PS);
	}

	bIsAbilitiesInitialized = true;
	GetWorldTimerManager().ClearTimer(InitTimerHandle);

	if (IsLocallyControlled())
	{
		if (APTWPlayerController* PlayerController = Cast<APTWPlayerController>(GetController()))
		{
			PlayerController->Server_NotifyReadyToPlay();
		}
	}
}

void APTWPlayerCharacter::EquipFirstWeapon(const FInputActionValue& Value)
{
	if (IsLocallyControlled())
	{
		ServerRPCEquipWeapon(0);
	}
}

void APTWPlayerCharacter::EquipSecondWeapon(const FInputActionValue& Value)
{
	if (IsLocallyControlled())
	{
		ServerRPCEquipWeapon(1);
	}
}

void APTWPlayerCharacter::UseActiveItem(const FInputActionValue& Value)
{
	ServerRPCUseActiveItem();
}

void APTWPlayerCharacter::UpdateNameTagText()
{
	if (!NameTagWidget) return;

	APTWPlayerState* PS = GetPlayerState<APTWPlayerState>();
	UPTWPlayerName* NameWidget = Cast<UPTWPlayerName>(NameTagWidget->GetUserWidgetObject());

	if (!PS || !NameWidget)
	{
		GetWorldTimerManager().SetTimer(NameTagRetryTimer, this, &APTWPlayerCharacter::UpdateNameTagText, 0.2f, false);
		return;
	}
	GetWorldTimerManager().ClearTimer(NameTagRetryTimer);

	FString Name;
	const FPTWPlayerData& PD = PS->GetPlayerData();
	if (!PD.PlayerName.IsEmpty())
	{
		Name = PD.PlayerName;
	}
	else
	{
		Name = PS->GetPlayerName();
	}

	NameWidget->SetPlayerName(Name);
}

void APTWPlayerCharacter::RegisterGameplayTagEvents()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(GameplayTags::State::Stasis, EGameplayTagEventType::AnyCountChange)
		.AddUObject(this, &APTWPlayerCharacter::OnStasisTagChanged);
		
		AbilitySystemComponent->RegisterGameplayTagEvent(GameplayTags::State::Stun, EGameplayTagEventType::AnyCountChange)
		.AddUObject(this, &APTWPlayerCharacter::OnMovelimit);
	}
}

void APTWPlayerCharacter::OnStasisTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	APTWPlayerController* PC = Cast<APTWPlayerController>(Controller);
	if (!PC) return;
	
	if (NewCount > 0)
	{
		PC->SetIgnoreLookInput(true);
		PC->SetIgnoreMoveInput(true);
		GetCharacterMovement()->StopMovementImmediately();
	}
	else
	{
		PC->ResetIgnoreLookInput();
		PC->ResetIgnoreMoveInput();
	}
}

void APTWPlayerCharacter::OnMovelimit(const FGameplayTag Tag, int32 NewCount)
{
	APTWPlayerController* PC = Cast<APTWPlayerController>(Controller);
	if (!PC) return;
	
	if (NewCount > 0)
	{
		PC->SetIgnoreMoveInput(true);
		GetCharacterMovement()->StopMovementImmediately();
	}
	else
	{
		PC->ResetIgnoreMoveInput();
	}
}

void APTWPlayerCharacter::OnRep_StealthMode()
{

}

void APTWPlayerCharacter::StartInitTimer()
{
	if (!GetWorldTimerManager().IsTimerActive(InitTimerHandle))
	{
		GetWorldTimerManager().SetTimer(InitTimerHandle, this, &APTWPlayerCharacter::InitCharacterState, 0.2f, true);
	}
}

void APTWPlayerCharacter::ClearLastAttacker()
{
	LastAttackerActor = nullptr;
}

void APTWPlayerCharacter::SetStealthMode(bool bSetStealthMode)
{
	bIsStealth = bSetStealthMode;
	OnRep_StealthMode();
}

void APTWPlayerCharacter::SetIceVisual(bool bEnable)
{
	MulticastSetIceVisual(bEnable);
}

void APTWPlayerCharacter::MulticastSetIceVisual_Implementation(bool bEnable)
{
	UMaterialInterface* OverlayMaterial = bEnable ? IceOverlayMaterial : nullptr;

	if (GetMesh())
	{
		GetMesh()->SetOverlayMaterial(OverlayMaterial);
	}

	if (Mesh1P)
	{
		Mesh1P->SetOverlayMaterial(OverlayMaterial);
	}

	if (Mesh1PHand)
	{
		Mesh1PHand->SetOverlayMaterial(OverlayMaterial);
	}
}

void APTWPlayerCharacter::ServerRPCUseActiveItem_Implementation()
{
	if (UPTWInventoryComponent* InventoryComponent = GetInventoryComponent())
	{
		InventoryComponent->UseActiveItem();
	}
}

void APTWPlayerCharacter::ServerRPCEquipWeapon_Implementation(int32 SelectIndex)
{
	if (UPTWInventoryComponent* InventoryComponent = GetInventoryComponent())
	{
		InventoryComponent->EquipWeapon(SelectIndex);
	}
}


void APTWPlayerCharacter::OnPlayerDataLoaded(const FPTWPlayerData& NewData)
{
	if (bHasGivenStartupItems || NewData.InventoryItemIDs.Num() == 0)
	{
		return;
	}

	APTWPlayerState* PS = GetPlayerState<APTWPlayerState>();
	if (!PS) return;

	if (UPTWItemSpawnManager* SpawnSys = GetWorld()->GetSubsystem<UPTWItemSpawnManager>())
	{
		if (APTWMiniGameMode* MiniGameMode = Cast<APTWMiniGameMode>(GetWorld()->GetAuthGameMode()))
		{
			if (!MiniGameMode->PlayerDeadCheck(GetController()))
			{
				SpawnSys->SpawnAndGiveItems(PS);
			}
		}
		
		bHasGivenStartupItems = true;
		PS->OnPlayerDataUpdated.RemoveDynamic(this, &APTWPlayerCharacter::OnPlayerDataLoaded);;
	}
}

