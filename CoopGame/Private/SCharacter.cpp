// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"

#include "SGameInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/NavMovementComponent.h"
#include "AI/Navigation/NavigationTypes.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "../CoopGame.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ASCharacter::ASCharacter() {
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	ZoomedFOV = 65.0f;
	ZoomInterSpeed = 20.0f;

	CurrentWeaponAttachSocketName = "WeaponSocket";
	StandbyWeaponAttachSocketName = "StandbyWeaponSocket";

	SprintSpeedMultiplier = 1.5;

	AimSpaceInterpolateSpeed = 15.0;
}

void ASCharacter::Destroyed() {
	DestroyWeapon();
}

// Called when the game starts or when spawned
// begin play runs everywhere on server or clients
void ASCharacter::BeginPlay() {
	Super::BeginPlay();
	DefaultFOV = CameraComp->FieldOfView;
	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);

	// initialize Weapon slots
	USGameInstance *GameInstance = Cast<USGameInstance>(GetGameInstance());

	if (GameInstance) {
		MainWeaponClass = GameInstance->PlayerMainWeaponClass;
		SecondaryWeaponClass = GameInstance->PlayerSecondaryWeaponClass;
	}

	if (MainWeaponClass) {
		ZoomedFOV = MainWeaponClass.GetDefaultObject()->ZoomedFOV;
	}

	// only run on server
	if (HasAuthority()) {
		// spawn a default weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		MainWeapon = GetWorld()->SpawnActor<ASWeapon>(MainWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator,
		                                                 SpawnParams);
		SecondaryWeapon = GetWorld()->SpawnActor<ASWeapon>(SecondaryWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator,
		                                                 SpawnParams);

		if (MainWeapon) {
			MainWeapon->SetOwner(this);
			MainWeapon->AttachToComponent(GetMesh(), 
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			                                 CurrentWeaponAttachSocketName);
			CurrentWeapon = MainWeapon;
		}

		if (SecondaryWeapon) {
			SecondaryWeapon->SetOwner(this);
			SecondaryWeapon->AttachToComponent(GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				StandbyWeaponAttachSocketName);
		}
	}
}


void ASCharacter::MoveForward(float Value) {
	AddMovementInput(GetActorForwardVector() * Value);
}


void ASCharacter::MoveRight(float Value) {
	AddMovementInput(GetActorRightVector() * Value);
}

void ASCharacter::BeginCrouch() {
	Crouch();
}

void ASCharacter::EndCrouch() {
	UnCrouch();
}

void ASCharacter::BeginSprint_Implementation() {
	if (CurrentWeapon) {
		if (CurrentWeapon->bIsFiring) return;
	}
	
	this->bIsSprinting = true;
	UCharacterMovementComponent* MovementComp = Cast<UCharacterMovementComponent>(this->GetCharacterMovement());
	MovementComp->MaxWalkSpeed *= SprintSpeedMultiplier;
}

void ASCharacter::ServerBeginSprint_Implementation() {
	BeginSprint();
}

void ASCharacter::EndSprint_Implementation() {
	if (!bIsSprinting) return;
	
	this->bIsSprinting = false;
	UCharacterMovementComponent* MovementComp = Cast<UCharacterMovementComponent>(this->GetCharacterMovement());
	MovementComp->MaxWalkSpeed /= SprintSpeedMultiplier;
}

void ASCharacter::ServerEndSprint_Implementation() {
	EndSprint();
}

void ASCharacter::BeginZoom() {
	bWantsToZoom = true;
}

void ASCharacter::EndZoom() {
	bWantsToZoom = false;
}

void ASCharacter::StartFire() {
	if (bIsReloading || bIsSprinting) return;
	
	if (CurrentWeapon) {
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::StopFire() {
	if (CurrentWeapon) {
		CurrentWeapon->StopFire();
	}
}

void ASCharacter::DestroyWeapon() {
	if (MainWeapon) {
		MainWeapon->Destroy();
	}

	if (SecondaryWeapon) {
		SecondaryWeapon->Destroy();
	}
}

void ASCharacter::UpdateAimSpace(float DeltaTime) {
	if (HasAuthority()) {
		FRotator TargetRotation = this->GetControlRotation();
		FRotator CurrentRotator = FRotator(AimPitch, AimYaw, 0);
		FRotator InterpedRotator = FMath::RInterpTo(CurrentRotator, TargetRotation, DeltaTime, AimSpaceInterpolateSpeed);

		float Roll, Pitch, Yaw;
		UKismetMathLibrary::BreakRotator(InterpedRotator, Roll, Pitch, Yaw);

		AimPitch = FMath::ClampAngle(Pitch, -90, 90);
		AimYaw = FMath::ClampAngle(Yaw, -90, 90);
	}
}

void ASCharacter::OnHealthChanged(USHealthComponent* OwningHealthComp, float TotalHealth, float CurrentHealth, float HealthDelta,
                                  const class UDamageType* DamageType, class AController* InsitigatedBy,
                                  AActor* DamageCauser) {
	if (CurrentHealth <= 0.0f && !bDied) {
		// DIE
		bDied = true;
		
		// stop movement
		GetMovementComponent()->StopMovementImmediately();
		StopFire();

		// stop collision
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// animate character death ( handled using event )

		// Detach controller
		DetachFromControllerPendingDestroy();
		SetLifeSpan(5.0f);

		PlayDeathSFX();
	}
}

void ASCharacter::PlayDeathSFX_Implementation() {
	if (DeathSFX != nullptr) {
		UGameplayStatics::PlaySound2D(this, DeathSFX);
	}
}


void ASCharacter::Reload_Implementation() {
	if (bIsSprinting) return;
	bIsReloading = true;
}

void ASCharacter::ServerReload_Implementation() {
	Reload();
}

bool ASCharacter::ServerReload_Validate() {
	return true;
}

void ASCharacter::TriggerServerChangeWeapon_Implementation() {
	if (WeaponSlotInUse == EWeaponSlotInUse::MainWeapon) {
		ServerChangeWeapon(SecondaryWeapon, EWeaponSlotInUse::SecondaryWeapon);
	} else {
		ServerChangeWeapon(MainWeapon, EWeaponSlotInUse::MainWeapon);
	}
}

void ASCharacter::ServerChangeWeapon_Implementation(ASWeapon *TargetWeapon, EWeaponSlotInUse WeaponSlot) {
	CurrentWeapon->AttachToComponent(GetMesh(), 
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		StandbyWeaponAttachSocketName);

	ASWeapon *PreviousWeapon = CurrentWeapon;

	CurrentWeapon = TargetWeapon;

	CurrentWeapon->AttachToComponent(GetMesh(),
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		CurrentWeaponAttachSocketName);

	ZoomedFOV = TargetWeapon->ZoomedFOV;

	WeaponSlotInUse = WeaponSlot;

	OnWeaponSwitched.Broadcast(PreviousWeapon, TargetWeapon);
}

// Called every frame
void ASCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterSpeed);
	CameraComp->SetFieldOfView(NewFOV);

	UpdateAimSpace(DeltaTime);
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	// takes care of looking up
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASCharacter::ServerReload);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASCharacter::ServerBeginSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASCharacter::ServerEndSprint);

	PlayerInputComponent->BindAction("SwitchWeapon", IE_Pressed, this, &ASCharacter::TriggerServerChangeWeapon);
}

FVector ASCharacter::GetPawnViewLocation() const {
	if (CameraComp) {
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

// this defines how to replicate 
void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, WeaponSlotInUse);
	DOREPLIFETIME(ASCharacter, bDied);
	DOREPLIFETIME(ASCharacter, AimYaw);
	DOREPLIFETIME(ASCharacter, AimPitch);
}
