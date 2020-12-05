// Fill out your copyright notice in the Description page of Project Settings.


#include "SPickActor.h"
#include "TimerManager.h"

// Sets default values
ASPickActor::ASPickActor()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(75.0f);
	RootComponent = SphereComp;

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetRelativeRotation(FRotator(90, 0.0f, 0.0f));
	DecalComp->DecalSize = FVector(64, 75, 75);
	DecalComp->SetupAttachment(RootComponent);

	CoolDownDuration = 10.0f;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ASPickActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority()) {
		Respawn();
	}
}

void ASPickActor::Respawn() {

	if (PowerupClass == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("PowerUpClass is nullptr in %s. Please update your blueprint"), *GetName());
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PowerupInstance = GetWorld()->SpawnActor<ASPowerupActor>(PowerupClass, GetTransform(), SpawnParams);

}

void ASPickActor::NotifyActorBeginOverlap(AActor *OtherActor) {
	Super::NotifyActorBeginOverlap(OtherActor);

	// @TODO: Grant a powerup to player if avilable

	if (HasAuthority() && PowerupInstance) {
		PowerupInstance->ActivatePowerup(OtherActor);
		PowerupInstance = nullptr;

		// set timer to respawn 
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ASPickActor::Respawn, CoolDownDuration);
	}
}

