// Fill out your copyright notice in the Description page of Project Settings.


#include "SEntryGameState.h"

void ASEntryGameState::TravelToWorld() {
	if (HasAuthority()) {
		GetWorld()->SeamlessTravel("Map1");
	}
}
