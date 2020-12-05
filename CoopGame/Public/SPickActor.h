// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "SPowerupActor.h"
#include "SPickActor.generated.h"

UCLASS()
class COOPGAME_API ASPickActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPickActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Componenets")
	USphereComponent *SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Componenets")
	UDecalComponent *DecalComp;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	TSubclassOf<ASPowerupActor> PowerupClass;

	UFUNCTION()
	void Respawn();

	ASPowerupActor *PowerupInstance;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	float CoolDownDuration;

	FTimerHandle TimerHandle_RespawnTimer;

public:
	virtual void NotifyActorBeginOverlap(AActor *OtherActor) override;



};
