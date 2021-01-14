// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

// OnHealthChangedEvent
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SevenParams(FOnHealthChangedSignature, USHealthComponent *, HealthComp, float, TotalHealth, float, CurrentHealth, float, HealthDelta, const class UDamageType *, DamageType, class AController *, InsitigatedBy, AActor *, DamageCauser);

UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "HealthComponent")
	float CurrentHealth;

	UFUNCTION()
	void OnRep_Health(float OldHealth);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthComponent")
	float TotalHealth;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor *DamagedActor, float Damage, const class UDamageType *DamageType, class AController *InsitigatedBy, AActor *DamageCauser);

	bool bIsDead;

public:	
	
	float GetHealth() const;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;
		
	UFUNCTION(BlueprintCallable, Category = "HealthComp")
	void Heal(float HealAmount);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HealthComponent")
	uint8 TeamNum;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "HealthComp")
	static bool IsFriendly(AActor *ActorA, AActor *ActorB);
};
