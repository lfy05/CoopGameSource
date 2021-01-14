// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "SWeapon.h"
#include "SHealthComponent.h"
#include "Sound/SoundCue.h"
#include "SCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponSwitched, ASWeapon *, PreviousWeapon, ASWeapon *, TargetWeapon);

UENUM()
enum class EWeaponSlotInUse: uint8 {
	MainWeapon,
	SecondaryWeapon
};

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter {
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

	virtual void Destroyed() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	/* Camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	bool bWantsToZoom;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	float DefaultFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.0, ClampMax = 100.0))
	float ZoomInterSpeed;

	void BeginZoom();

	void EndZoom();

	/* Movement Controls */
	void MoveForward(float Value);

	void MoveRight(float Value);

	void BeginCrouch();

	void EndCrouch();

	UFUNCTION(NetMulticast, Reliable)
	void BeginSprint();

	UFUNCTION(Server, Reliable)
	void ServerBeginSprint();

	UFUNCTION(NetMulticast, Reliable)
	void EndSprint();

	UFUNCTION(Server, Reliable)
	void ServerEndSprint();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bIsSprinting;

	UPROPERTY(EditDefaultsOnly)
	float SprintSpeedMultiplier;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float AimYaw;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float AimPitch;

	UPROPERTY(EditDefaultsOnly)
	float AimSpaceInterpolateSpeed;
	
	void UpdateAimSpace(float DeltaTime);

	/* Player State */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealthComp;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, 
		float TotalHealth, 
		float CurrentHealth, 
		float HealthDelta,
        const class UDamageType* DamageType, 
		class AController* InsitigatedBy,
        AActor* DamageCauser);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bDied;

	UFUNCTION(NetMulticast, Reliable)
	void PlayDeathSFX();

	/* Weapon */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
	EWeaponSlotInUse WeaponSlotInUse;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon") // remember to override GetLifetimeReplicatedRrops
	ASWeapon* CurrentWeapon;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
	ASWeapon *MainWeapon;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
	ASWeapon *SecondaryWeapon;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<ASWeapon> MainWeaponClass;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<ASWeapon> SecondaryWeaponClass;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName CurrentWeaponAttachSocketName;

	UPROPERTY(VisibleDefaultsOnly, Category = "Plauyer")
	FName StandbyWeaponAttachSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	USoundCue* DeathSFX;

	UPROPERTY(EditDefaultsOnly)
	int reloadTime;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	bool bIsReloading;

	UFUNCTION(NetMulticast, Reliable)
	void Reload();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReload();

	UFUNCTION(Server, Reliable)
	void TriggerServerChangeWeapon();

	UFUNCTION(NetMulticast, Reliable)
	void ServerChangeWeapon(ASWeapon *TargetWeapon, EWeaponSlotInUse WeaponSlot);

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnWeaponSwitched OnWeaponSwitched;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/* Movement Controls */
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const;

	/* Weapon */
	UFUNCTION(BlueprintCallable, Category = "Player")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StopFire();

	void DestroyWeapon();
};
