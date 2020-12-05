// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "SHealthComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"
#include "STrackerBot.generated.h"

UCLASS()
class COOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Componenets")
	UStaticMeshComponent *MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Componenets")
	USHealthComponent *HealthComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Componenets")
	USphereComponent *SphereComp;

	UFUNCTION()
	void HandleTakeDamage(USHealthComponent *OwningHealthComp, float Health, float HealthDelta, const class UDamageType *DamageType, class AController *InsitigatedBy, AActor *DamageCauser);

	FVector GetNextPathPoint();

	// Next way point in navigation
	FVector NextPathPoint;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bUseVelocityChange;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RequiredDistanceToTarget;

	// Dynamic material to pulse on damage
	UMaterialInstanceDynamic *MatInst;

	void SelfDestruct();

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	UParticleSystem *ExplosionEffect;

	bool bExploded;

	bool bStartedSelfDestruction;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float SelfDamageInterval;

	FTimerHandle TimerHandle_SelfDamage;

	void DamageSelf();

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	USoundCue *SelfDestructSound;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	USoundCue *ExplodeSound;

	void OnCheckNearbyBots();

	int32 PowerLevel;

	FTimerHandle TimerHandle_RefreshPath;

	void RefreshPath();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void NotifyActorBeginOverlap(AActor *OtherActor) override;
};
