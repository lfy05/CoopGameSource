// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "../CoopGame.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("draw debug lines for weapons"),
	ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "Muzzle";
	TracerTargetName = "BeamEnd";
	BaseDamage = 20.0f;
	BulletSpread = 2.0f;
	RateOfFire = 600; // bullets per minute

	// set this as a replicated actor. Remember to also tick replicates in the blueprint (when double loading, double check variables)
	SetReplicates(true);
	// SetIsReplicatedByDefault(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;

	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	AudioComp->SetupAttachment(RootComponent);
	// AudioComp->SetIsReplicated(true);
}

void ASWeapon::BeginPlay() {
	Super::BeginPlay();

	BulletsInClip = DefaultClipSize;

	// set time between shots
	TimeBetweenShots = 60 / RateOfFire;

	OnBulletCountChanged.Broadcast(BulletsInClip, DefaultClipSize);
}



void ASWeapon::Fire() {
	// trace the world from pawn eyes to crosshair location (Center screen)

	if (BulletsInClip <= 0) {
		StopFire();
		return;
	}
	
	BulletsInClip--;
	OnBulletCountChanged.Broadcast(BulletsInClip, DefaultClipSize);

	// make sure Fire() runs on server
	if (!HasAuthority()) {
		ServerFire();
	}

	PlayFireSoundEffect();

	AActor *MyOwner = GetOwner();
	if (MyOwner) {
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation); // this uses Character's GetPawnViewLocation and base eye height. To change the actor eye point to another component, override GetPawnViewLocation in SCharacter
		
		FVector ShotDirection = EyeRotation.Vector();

		// bullet spread
		float HalfRad = FMath::DegreesToRadians(BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

		FVector TraceEnd = EyeLocation + (EyeRotation.Vector() * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bReturnPhysicalMaterial = true;
		QueryParams.bTraceComplex = true;

		// Particle "Target" parameter
		FVector TracerEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType = SurfaceType_Default;

		FHitResult Hit;
		// Use ECC_Visibility Channel will return all objects that the ray collides with
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams)) { // collision weapon defined in CoopGame.h
			// Blocking hit! Process Damage
			AActor *HitActor = Hit.GetActor();

			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get()); // weak reference, not guaranteed to stay in RAM, could get deleted even if DetermineSurfaceType is using if

			// headshot damage increase
			float ActualDamage = BaseDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE) {
				ActualDamage *= 4.0f;
			}

			// damage should be applied on server. Point damage is internally replication supported
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);

			PlayImpactEffect(SurfaceType, Hit.ImpactPoint);
			
			TracerEndPoint = Hit.ImpactPoint;

			
		}
		
		if (DebugWeaponDrawing) {
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
		}

		PlayFireEffects(TracerEndPoint);

		// replicate trace
		if (HasAuthority()) {
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}

		LastFireTime = GetWorld()->TimeSeconds;
	}
}

void ASWeapon::StartFire() {
	if (BulletsInClip <= 0) {
		StopFire();
		return;
	}

	bIsFiring = true;

	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f); // need to make sure it is not negative
	
	// UE_LOG(LogTemp, Log, TEXT("Playing firing SFX"));

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
	
	Fire();
}

void ASWeapon::StopFire_Implementation() {
	StopFireSoundEffect();

	// UE_LOG(LogTemp, Log, TEXT("Stopping firing SFX"));
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);

	bIsFiring = false;
}

void ASWeapon::Reload_Implementation() {
	
	BulletsInClip = DefaultClipSize;
	OnBulletCountChanged.Broadcast(BulletsInClip, DefaultClipSize);
}

void ASWeapon::PlayFireEffects(FVector TraceEndPoint) {
	if (MuzzleEffect) {
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect) {
		// actual location required
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		// UParticleSystemComponent *TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		UNiagaraComponent *TracerComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TracerEffect, MuzzleLocation, 
			(TraceEndPoint - MuzzleLocation).Rotation());
		// TracerComp->SetNiagaraVariableLinearColor(FString(TEXT("User.Color")), TracerColor);
		//if (TracerComp) {
			//UE_LOG(LogTemp, Log, TEXT("Setting End"));
			//TracerComp->
			//UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		//}
	}

	APawn *MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner) {
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC) {
			// PC->ClientPlayCameraShake(FireCamShake);

			PC->ClientStartCameraShake(FireCamShake);
		}
	}
}

void ASWeapon::PlayImpactEffect(EPhysicalSurface SurfaceType, FVector ImpactPoint) {
	UParticleSystem *SelectedEffect = nullptr;
	switch (SurfaceType) {
		case SURFACE_FLESHDEFAULT:	// flesh default
			SelectedEffect = DefaultImpactEffect;
			break;

		case SURFACE_FLESHVULNERABLE:
			SelectedEffect = FleshImpactEffect;
			break;

		default:
			SelectedEffect = DefaultImpactEffect;
			break;
	}

	if (SelectedEffect) {
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());

	}
}

void ASWeapon::ServerFire_Implementation() {
	Fire();
}

bool ASWeapon::ServerFire_Validate() {
	// validate the code to see if there is something wrong
	// if false, the requesting client would be disconnected
	return true;
}

void ASWeapon::PlayFireSoundEffect_Implementation() {
	// UE_LOG(LogTemp, Log, TEXT("Multicast firingSFX"));
	AudioComp->Play(0.0f);
}

bool ASWeapon::PlayFireSoundEffect_Validate() {
	return true;
}

void ASWeapon::StopFireSoundEffect_Implementation() {
	UE_LOG(LogTemp, Log, TEXT("Multicast stop firingSFX"));
	//AudioComp->Stop();
}

bool ASWeapon::StopFireSoundEffect_Validate() {
	return true;
}

void ASWeapon::OnRep_HitScanTrace() {
	// play cosmetic FX
	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffect(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);

}

void ASWeapon::PostInitializeComponents() {
	Super::PostInitializeComponents();

	if (FiringSFX) {
		AudioComp->SetSound(FiringSFX);
		UE_LOG(LogTemp, Log, TEXT("SWeapon: Firing SFX set"));
		AudioComp->SetVolumeMultiplier(2.0f);
	} else {
		UE_LOG(LogTemp, Log, TEXT("SWeapon: No FiringSFX set"));
		
	}

}


// this function doesn't need to be declared in the header file. It is automatically generated CONST is very important!!!!
void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// The requesting (sending) client doesn't need to receive what it has sent again (COND_SkipOwner)
	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}