// Fill out your copyright notice in the Description page of Project Settings.


#include "SGrenade.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

void ASGrenade::Fire() {
	// get owner
	AActor *owner = GetOwner();
	if (owner && GrenadeClass) {
		// get camera viewpoint
		FVector EyeLocation;
		FRotator EyeRotation;
		owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		// spawn a grenade
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(GrenadeClass, MuzzleLocation, EyeRotation, SpawnParams);
	}


}
