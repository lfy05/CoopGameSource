// Fill out your copyright notice in the Description page of Project Settings.


#include "SPvPPlayerController.h"

#include "SPvPPlayerState.h"
#include "GameFramework/GameModeBase.h"


void ASPvPPlayerController::UpdateDisplayName(FString NewDisplayName) {
	if (GetWorld()->GetAuthGameMode()) {
		GetWorld()->GetAuthGameMode()->ChangeName(this, NewDisplayName, false);
	}
}
