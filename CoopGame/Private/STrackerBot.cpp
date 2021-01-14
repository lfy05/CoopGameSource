// Fill out your copyright notice in the Description page of Project Settings.


#include "STrackerBot.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "SCharacter.h"
#include "Components/PrimitiveComponent.h"

static int32 DebugTrackerBotDrawing = 0;
FAutoConsoleVariableRef CVARDebugTrackerBotDrawing(TEXT("COOP.DebugTrackerBot"),
	DebugTrackerBotDrawing,
	TEXT("draw debug lines for TrackerBot"),
	ECVF_Cheat);

// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // prevent unwanted collision
	SphereComp->SetupAttachment(RootComponent);
	
	bUseVelocityChange = false;
	MovementForce = 1000;
	RequiredDistanceToTarget = 100;

	ExplosionRadius = 60;
	ExplosionDamage = 350;
	SelfDamageInterval = 0.25f;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority()) {
		// find initial move-to
		NextPathPoint = GetNextPathPoint();
		
		// set timer to check nearby bots
		FTimerHandle TimerHandle_CheckPowerLevel;
		GetWorldTimerManager().SetTimer(TimerHandle_CheckPowerLevel, this, &ASTrackerBot::OnCheckNearbyBots, 1.0f, true);
	}
}

void ASTrackerBot::HandleTakeDamage(USHealthComponent *OwningHealthComp, 
		float TotalHealth, 
		float CurrentHealth, 
		float HealthDelta, 
		const class UDamageType *DamageType, 
		class AController *InsitigatedBy, 
		AActor *DamageCauser) {
	
	// Explode on hitpoints == 0

	// @TODO: Pulse the material on hit
	if (MatInst == nullptr) {
		// make sure it is a DYNAMIC material instance. Otherwise a parameter change would change all instances with the material
		MatInst = MeshComp->CreateDynamicMaterialInstance(0, MeshComp->GetMaterial(0));
	}

	if (MatInst) {
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	// UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());

	if (CurrentHealth <= 0.0f) {
		SelfDestruct();
	}
}

FVector ASTrackerBot::GetNextPathPoint() {
	AActor *BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX;

	// hack to get player location
	for (FConstPawnIterator it = GetWorld()->GetPawnIterator(); it; ++it) {
		APawn *TestPawn = it->Get();
		if (TestPawn == nullptr || USHealthComponent::IsFriendly(TestPawn, this)) {
			continue;
		}

		USHealthComponent *TestPawnHealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (TestPawnHealthComp && TestPawnHealthComp->GetHealth() > 0.0f) {
			float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();

			if (Distance < NearestTargetDistance) {
				BestTarget = TestPawn;
				NearestTargetDistance = Distance;
			}
		}
	}

	if (BestTarget) {
		UNavigationPath *NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);

		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ASTrackerBot::RefreshPath, 5.0f, false);

		if (NavPath && NavPath->PathPoints.Num() > 1) {
			// return the next way point
			return NavPath->PathPoints[1];
		}
	}

	

	// failed to find path
	return GetActorLocation();
}

void ASTrackerBot::SelfDestruct() {
	if (bExploded) {
		return;
	}

	bExploded = true;

	// emitter and sound shouldn't be running on dedicated servers
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (HasAuthority()) {
		TArray<AActor *> IgnoredActors;
		IgnoredActors.Add(this);

		// calculate damage
		float ActualDamage = ExplosionDamage + (ExplosionDamage * PowerLevel);

		// apply damage
		UE_LOG(LogTemp, Log, TEXT("TrackerBot Applied Damage"));
		UGameplayStatics::ApplyRadialDamage(this, ActualDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

		if (DebugTrackerBotDrawing)
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);

		// delete actor immediately
		// Destroy(); destroy too soon would not allow client to see the emitter
		SetLifeSpan(2.0f); // self deleted after lifespan expired.
	}
}

void ASTrackerBot::DamageSelf() {
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

void ASTrackerBot::OnCheckNearbyBots() {
	// distance to check for nearby bots
	const float Radius = 600;

	// craete temporary collision shape for overlaps
	FCollisionShape CollShape;
	CollShape.SetSphere(Radius);

	// only find Pawns
	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);

	if (DebugTrackerBotDrawing)
		DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 12, FColor::White, false, 1.0f);

	int32 NrOfBots = 0;
	// loop over the results
	for (FOverlapResult Result : Overlaps) {
		ASTrackerBot *Bot = Cast<ASTrackerBot>(Result.GetActor());

		if (Bot && Bot != this) {
			NrOfBots++;
		}
	}

	const int32 MaxPowerLevel = 4;

	PowerLevel = FMath::Clamp(NrOfBots, 0, MaxPowerLevel);

	if (MatInst == nullptr) {
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst) {
		float Alpha = PowerLevel / (float) MaxPowerLevel;
		MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
	}
}

void ASTrackerBot::RefreshPath() {
	NextPathPoint = GetNextPathPoint();
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority()) {
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if (DistanceToTarget <= RequiredDistanceToTarget) {
			// very close to player
			NextPathPoint = GetNextPathPoint();
			if (DebugTrackerBotDrawing)
				DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached");
		} else {
			// Keep moving towards next waypoint by adding force
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();

			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			if (DebugTrackerBotDrawing)
				DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
		}

		if (DebugTrackerBotDrawing)
			DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 1.0f);
	}
}

// Called to bind functionality to input
void ASTrackerBot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ASTrackerBot::NotifyActorBeginOverlap(AActor *OtherActor) {
	Super::NotifyActorBeginOverlap(OtherActor);

	if (!bStartedSelfDestruction && !bExploded) {
		ASCharacter *PlayerPawn = Cast<ASCharacter>(OtherActor);
		if (PlayerPawn && !USHealthComponent::IsFriendly(OtherActor, this)) {
			// overlapped with a player

			// set a timer that count down till trigger self ON SERVER ONLY

			if (HasAuthority()) {
				// start self-destruction sequence
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
			}
			
			bStartedSelfDestruction = true;

			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
		}
	}
}

