// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerState.h"


void ASPlayerState::AddScore_Implementation(float ScoreDelta) {
	SetScore(GetScore() + ScoreDelta);
	OnScoreUpdated.Broadcast(GetScore());
	UE_LOG(LogTemp, Log, TEXT("Score added"));
}
