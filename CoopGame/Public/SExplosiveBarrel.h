// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Materials/MaterialInterface.h"
#include "Particles/ParticleSystem.h"
#include "SExplosiveBarrel.generated.h"

UCLASS()
class COOPGAME_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USHealthComponent *HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent *MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Componenets")
	URadialForceComponent *RadialForceComp;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent *OwningHealthComp, float Health, float HealthDelta, const class UDamageType *DamageType,
		class AController *InstigatedBy, AActor *DamageCause);

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem *ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UMaterialInterface *ExplodedMaterial;

	UPROPERTY(ReplicatedUsing = OnRep_Explode)
	bool bExploed;

	UFUNCTION()
	void OnRep_Explode();

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	float ExplosionImpulse;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
