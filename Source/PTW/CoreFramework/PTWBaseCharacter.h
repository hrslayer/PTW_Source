// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "PTWCombatInterface.h"
#include "PTWBaseCharacter.generated.h"

USTRUCT(BlueprintType)
struct FCharacterCustomizationInfo
{
	GENERATED_BODY()

	UPROPERTY() FName EquippedUpperAddonID;
	UPROPERTY() FName EquippedBackAddonID;
	UPROPERTY() FName EquippedBodyID;
	UPROPERTY() FName EquippedEyewearID;
	UPROPERTY() FName EquippedGlovesID;
	UPROPERTY() FName EquippedHairID;
	UPROPERTY() FName EquippedHatID;
	UPROPERTY() FName EquippedHeadID;
	UPROPERTY() FName EquippedLowerID;
	UPROPERTY() FName EquippedShoesID;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterDeathSignature, AActor*, DeadActor, AActor*, KillerActor);

class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayAbility;
class UGameplayEffect;
class UPTWReactorComponent;


UCLASS()
class PTW_API APTWBaseCharacter : public ACharacter, public IAbilitySystemInterface, public IPTWCombatInterface
{
	GENERATED_BODY()


public:
	// 1. 생성자 (Constructor)
	APTWBaseCharacter();

	UFUNCTION(BlueprintPure)
	bool IsDead() const;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/*  ReactorComponet에서 Death 로직 구현됨 */
	virtual void HandleDeath(AActor* Attacker);

	FORCEINLINE UPTWReactorComponent* GetReactorComponent() const { return ReactorComponent; }
	
	/*CombatInterface 구현*/
	virtual float GetDamageMultiplier(const FName& BoneName) const override;
	
	virtual void RemoveEffectWithTag(const FGameplayTag& TagToRemove) override;
	
	virtual void ApplyGameplayEffectToSelf(TSubclassOf<class UGameplayEffect> EffectClass, float Level, FGameplayEffectContextHandle Context) override;
	
	virtual void ApplyGameplayEffectWithDuration(TSubclassOf<class UGameplayEffect> EffectClass, 
		float Level, 
		float Duration, 
		FGameplayEffectContextHandle Context) override;

	UFUNCTION(BlueprintCallable, Category = "Customization")
	void LoadLocalCustomizationAndSendToServer();

	// 실제로 메쉬를 데이터 테이블에서 찾아서 입히는 함수
	void ApplyCustomizationFromInfo(const FCharacterCustomizationInfo& Info);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void InitAbilityActorInfo();
	void GiveDefaultAbilities();
	void ApplyDefaultEffects();

private:

public:

protected:
	UPROPERTY(EditAnywhere, Category = "GAS|Default")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Default")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;
	UPROPERTY(Transient)
	TArray<TSubclassOf<UGameplayAbility>> AdditionalAbilities;
	UPROPERTY(Transient)
	TArray<TSubclassOf<UGameplayEffect>> AdditionalEffects;
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPTWReactorComponent> ReactorComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SK_UpperAddon;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SK_BackAddon;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SK_Eyewear;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SK_Gloves;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SK_Hair;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SK_Hat;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SK_Head;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SK_Lower;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SK_Shoes;

	UPROPERTY(ReplicatedUsing = OnRep_CustomizationInfo)
	FCharacterCustomizationInfo CustomizationInfo;

	UFUNCTION()
	void OnRep_CustomizationInfo();

	UFUNCTION(Server, Reliable)
	void Server_SetCustomizationInfo(FCharacterCustomizationInfo NewInfo);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnCharacterDeathSignature OnCharacterDied;

};
