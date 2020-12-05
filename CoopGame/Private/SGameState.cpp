// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

void ASGameState::OnRep_WaveState(EWaveState OldState) {
	WaveStateChanged(WaveState, OldState);
}

void ASGameState::OnRep_WaveCount() {
	OnWaveCountUpdated.Broadcast(WaveCount);
}

void ASGameState::SetWaveState(EWaveState NewState) {
	if (HasAuthority()) {
		EWaveState OldState = WaveState;

		// marking WaveState with ReplicatedUsing only triggers OnRep on the client, not the server. Have to manually call it on the server
		WaveState = NewState;
		OnRep_WaveState(OldState);
	}
}

EWaveState ASGameState::GetWaveState() {
	return WaveState;
}

int32 ASGameState::GetWaveCount() {
	return WaveCount;
}

void ASGameState::SetWaveCount(int32 NewWaveCount) {
	WaveCount = NewWaveCount;
}

void ASGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGameState, WaveCount);
	DOREPLIFETIME(ASGameState, WaveState);
}

void ASGameState::PlayWaveStartSFX_Implementation() {
	if (WaveStartSFX != nullptr) {
		UGameplayStatics::PlaySound2D(this, WaveStartSFX);
	} else {
		UE_LOG(LogTemp, Log, TEXT("WaveStartSFX is not assigned"));
	}
}

void ASGameState::PlayGameOverSFX_Implementation() {
	if (WaveStartSFX != nullptr) {
		UGameplayStatics::PlaySound2D(this, GameOverSFX);
	} else {
		UE_LOG(LogTemp, Log, TEXT("GameOverSFX is not assigned"));
	}
}