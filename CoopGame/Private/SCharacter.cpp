// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/NavMovementComponent.h"
#include "AI/Navigation/NavigationTypes.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "../CoopGame.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASCharacter::ASCharacter()
{
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

	WeaponAttachSocketName = "WeaponSocket";
}

// Called when the game starts or when spawned
// begin play runs everywhere on server or clients
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	DefaultFOV = CameraComp->FieldOfView;
	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);

	// only run on server
	if (HasAuthority()) {
		// spawn a default weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (CurrentWeapon) {
			CurrentWeapon->SetOwner(this);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
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

void ASCharacter::BeginZoom() {
	bWantsToZoom = true;
}

void ASCharacter::EndZoom() {
	bWantsToZoom = false;
}

void ASCharacter::StartFire() {
	if (CurrentWeapon) {
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::StopFire() {
	if (CurrentWeapon) {
		CurrentWeapon->StopFire();
	}
}



void ASCharacter::OnHealthChanged(USHealthComponent *OwningHealthComp, float Health, float HealthDelta, const class UDamageType *DamageType, class AController *InsitigatedBy, AActor *DamageCauser) {
	if (Health <= 0.0f && !bDied) {
		// DIE
		bDied = true;

		// stop weapon fire
		// UE_LOG(LogTemp, Log, TEXT("Stopped firing due to death"));
		

		// stop movement
		GetMovementComponent()->StopMovementImmediately();
		StopFire();

		// stop collision
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// animate character death ( handled using event )

		// Detach controller
		DetachFromControllerPendingDestroy();
		SetLifeSpan(10.0f);

		PlayDeathSFX();
	}
}

void ASCharacter::PlayDeathSFX_Implementation() {
	if (DeathSFX != nullptr) {
		UGameplayStatics::PlaySound2D(this, DeathSFX);
	}
}



void ASCharacter::Reload_Implementation() {
	GetMovementComponent()->StopMovementImmediately();

	PlayAnimMontage(ReloadingMontage);

	if (CurrentWeapon) {
		CurrentWeapon->Reload();
	}
}

void ASCharacter::ServerReload_Implementation() {
	Reload();
}

bool ASCharacter::ServerReload_Validate() {
	return true;
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterSpeed);
	CameraComp->SetFieldOfView(NewFOV);
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
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
}

FVector ASCharacter::GetPawnViewLocation() const {
	if (CameraComp) {
		return CameraComp->GetComponentLocation();
	} 

	return Super::GetPawnViewLocation();
}

// this defines how to replicate 
void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDied);
}