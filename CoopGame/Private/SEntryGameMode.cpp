// Fill out your copyright notice in the Description page of Project Settings.


#include "SEntryGameMode.h"

void ASEntryGameMode::TravelToWorld(FString InURL) {
	if (HasAuthority()) {
		GetWorld()->ServerTravel(InURL, true);
	}
	
}
