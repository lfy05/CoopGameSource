// Fill out your copyright notice in the Description page of Project Settings.


#include "SHealthComponent.h"

#include "SCharacter.h"
#include "Net/UnrealNetwork.h"
#include "SGameMode.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	TotalHealth = 100;
	CurrentHealth = TotalHealth;
	// SetIsReplicated(true);
	SetIsReplicatedByDefault(true);

	//TeamNum = 255;

	bIsDead = false;
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// only work if we are server
	if (GetOwner()->HasAuthority()) {
		AActor *MyOwner = GetOwner();
		if (MyOwner) {
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}
	}	
}

void USHealthComponent::OnRep_Health(float OldHealth) {
	float Damage = CurrentHealth - OldHealth;

	OnHealthChanged.Broadcast(this, TotalHealth, CurrentHealth, Damage, nullptr, nullptr, nullptr);
}

void USHealthComponent::HandleTakeAnyDamage(AActor *DamagedActor, float Damage, const class UDamageType *DamageType, class AController *InsitigatedBy, AActor *DamageCauser) {
	if (Damage <= 0.0f || bIsDead) {
		return;
	}

	// check if friendly
	if (DamageCauser != DamagedActor && IsFriendly(DamagedActor, DamageCauser)) {
		return;
	}

	// update health clamped
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, TotalHealth);

	//UE_LOG(LogTemp, Log, TEXT("CurrentHealth Changed: %s"), *FString::SanitizeFloat(CurrentHealth));

	bIsDead = CurrentHealth <= 0.0f;

	OnHealthChanged.Broadcast(this, TotalHealth, CurrentHealth, Damage, DamageType, InsitigatedBy, DamageCauser);

	if (bIsDead) {
		// destroy weapons
		ASCharacter *Owner = Cast<ASCharacter>(GetOwner());
		if (Owner) {
			Owner->DestroyWeapon();
		}
		
		ASGameMode *GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
		if (GM) {
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InsitigatedBy);
		}
	} 
}

float USHealthComponent::GetHealth() const {
	return CurrentHealth;
}

void USHealthComponent::Heal(float HealAmount) {
	// healing less than 0 or player already dead
	if (HealAmount < 0.0f || CurrentHealth <= 0.0f) {
		return;
	}

	CurrentHealth += FMath::Clamp(CurrentHealth + HealAmount, 0.0f, TotalHealth);

	// UE_LOG(LogTemp, Log, TEXT("CurrentHealth Changed"), *FString::SanitizeFloat(CurrentHealth), *FString::SanitizeFloat(HealAmount));

	OnHealthChanged.Broadcast(this, TotalHealth, CurrentHealth, -HealAmount, nullptr, nullptr, nullptr);
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USHealthComponent, CurrentHealth);
}

bool USHealthComponent::IsFriendly(AActor *ActorA, AActor *ActorB) {
	if (ActorA == nullptr || ActorB == nullptr) {
		return true;
	}

	USHealthComponent *HealthCompA = Cast<USHealthComponent>(ActorA->GetComponentByClass(USHealthComponent::StaticClass()));
	USHealthComponent *HealthCompB = Cast<USHealthComponent>(ActorB->GetComponentByClass(USHealthComponent::StaticClass()));

	if (HealthCompA == nullptr || HealthCompB == nullptr) {
		return true;
	}

	return HealthCompA->TeamNum == HealthCompB->TeamNum;
}