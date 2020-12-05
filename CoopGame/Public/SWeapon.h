// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "Camera/CameraShake.h"
#include "Engine/EngineTypes.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "SWeapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBulletCountChanged, int32, NumBulletsInClip);

// contains information of a single hitscan weapon linetrace
USTRUCT()
struct FHitScanTrace {
	GENERATED_BODY()

public:
	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;

};

UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

	//UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();

	void StartFire();

	UFUNCTION(NetMulticast, Reliable)
	void StopFire();

	UFUNCTION(NetMulticast, Reliable)
	void Reload();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 DefaultClipSize;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componenets")
	USkeletalMeshComponent *MeshComp;

	void PlayFireEffects(FVector TraceEndPoint);

	void PlayImpactEffect(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem *MuzzleEffect;
 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem *DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem *FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem *TracerEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> FireCamShake;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	UFUNCTION(Server, Reliable, WithValidation) // Server: won't happen on client, request get pushed to server, Reliable: eventually happen on server
	void ServerFire();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void PlayFireSoundEffect();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void StopFireSoundEffect();

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFireTime;

	// RPM: bullet per minute by weapon
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	// bullet spread in degrees
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin=0.0))
	float BulletSpread;

	// derived from rate of fire
	float TimeBetweenShots;
	
	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace) // everytime OnRep_HitScanTrace is called, the value gets replicated
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

	UAudioComponent *AudioComp;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	USoundCue *FiringSFX;

	void PostInitializeComponents() override;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 BulletsInClip;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnBulletCountChanged OnBulletCountChanged;
};
