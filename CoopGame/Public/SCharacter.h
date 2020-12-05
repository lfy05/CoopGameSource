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

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void BeginCrouch();

	void EndCrouch();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent *CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent *SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent *HealthComp;

	bool bWantsToZoom;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	float DefaultFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.0, ClampMax = 100.0))
	float ZoomInterSpeed;

	void BeginZoom();

	void EndZoom();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon") // remember to override GetLifetimeReplicatedRrops
	ASWeapon *CurrentWeapon;

	
	UFUNCTION()
	void OnHealthChanged(USHealthComponent *OwningHealthComp, float Health, float HealthDelta, const class UDamageType *DamageType, class AController *InsitigatedBy, AActor *DamageCauser);

	// pawn died previously
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bDied;
	
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> StarterWeaponClass;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	USoundCue *DeathSFX;

	UFUNCTION(NetMulticast, Reliable)
	void PlayDeathSFX();

	UFUNCTION(NetMulticast, Reliable)
	void Reload();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReload();

	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UAnimMontage *ReloadingMontage;

	UAnimInstance *AnimInstance;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StopFire();
};
