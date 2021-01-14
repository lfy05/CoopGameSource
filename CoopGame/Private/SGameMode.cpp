// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameMode.h"
#include "GameFramework/Actor.h"
#include "SHealthComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "SPlayerState.h"
#include "Kismet/GameplayStatics.h"


ASGameMode::ASGameMode() {
	TimeBetweenWaves = 2.0f;

	GameStateClass = ASGameState::StaticClass();
	PlayerStateClass = ASPlayerState::StaticClass();


	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
}

void ASGameMode::StartWave() {
	ASGameState *GS = GetGameState<ASGameState>();
	if (ensureAlways(GS)) {
		GS->SetWaveCount(GS->GetWaveCount() + 1);
		GS->PlayWaveStartSFX();
	}

	numOfBotsToSpawn = GS->GetWaveCount() * SpawnRateMultiplier + SpawnRateAdder;

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElasped, 1.0f, true, 0.0f);

	SetWaveState(EWaveState::WaveInProgress);
}

void ASGameMode::EndWave() {
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
	
	// check if game over
	ASGameState *GS = GetGameState<ASGameState>();
	if (GS->GetWaveState() == EWaveState::GameOver) {
		return;
	}

	PrepareForNextWave();

	SetWaveState(EWaveState::WaitingToComplete);
}

void ASGameMode::PrepareForNextWave() {

	ASGameState *GS = GetGameState<ASGameState>();
	EWaveState CurrentWaveState = GS->GetWaveState();

	if (CurrentWaveState == EWaveState::GameOver) return;
	
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStarts, this, &ASGameMode::StartWave, TimeBetweenWaves, false);

	SetWaveState(EWaveState::WaitingToStart);

	RestartDeadPlayers();
}

void ASGameMode::CheckWaveState() {
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStarts);

	// do nothing if the previous wave is not done
	if (numOfBotsToSpawn > 0 || bIsPreparingForWave) {
		return;
	}

	bool bIsAnyBotAlive = false;

	// check whether or not there are any bots left
	for (FConstPawnIterator it = GetWorld()->GetPawnIterator(); it; ++it) {
		APawn *TestPawn = it->Get();
		if (TestPawn == nullptr || TestPawn->IsPlayerControlled()) {
			continue;
		}

		USHealthComponent *HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (HealthComp && HealthComp->GetHealth() > 0.0f) {
			bIsAnyBotAlive = true;
			break;
		}
	}

	if (!bIsAnyBotAlive) {
		SetWaveState(EWaveState::WaveComplete);
		PrepareForNextWave();
	}
}

void ASGameMode::CheckAnyPlayerAlive() {
	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; ++it) {
		APlayerController *PC = it->Get();
		if (PC && PC->GetPawn()) {
			APawn *MyPawn = PC->GetPawn();
			USHealthComponent *HealthComp = Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		
			// ensure must succeed. Otherwise break the code. Prevent confusing debugging
			if (ensure(HealthComp) && HealthComp->GetHealth() > 0.0f) {
				// a player is still alive
				return;
			}
		}
	}

	// No Player Alive
	GameOver();

}

void ASGameMode::GameOver() {
	
	SetWaveState(EWaveState::GameOver);

	EndWave();

	ASGameState *GS = GetGameState<ASGameState>();
	if (ensureAlways(GS)) {
		GS->PlayGameOverSFX();
		GS->TriggerGameOverEvent();
	}

	// reload the level
	// UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), true);
	// RestartGame();

	//@TODO: finish up the match, presents "game over" to players
	// UE_LOG(LogTemp, Log, TEXT("GAME OVER! Player Died"));
	
}

void ASGameMode::SetWaveState(EWaveState NewState) {
	ASGameState *GS = GetGameState<ASGameState>();
	if (ensureAlways(GS)) {
		GS->SetWaveState(NewState);
	}
}

void ASGameMode::RestartDeadPlayers() {
	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; ++it) {
		APlayerController *PC = it->Get();
		if (PC && PC->GetPawn() == nullptr) {
			RestartPlayer(PC);
		}
	}
}

void ASGameMode::StartPlay() {
	Super::StartPlay();

	SetWaveState(EWaveState::WaitingToStart);

	PrepareForNextWave();
}

void ASGameMode::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	ASGameState *GS = GetGameState<ASGameState>();
	EWaveState CurrentWaveState = GS->GetWaveState();

	if (CurrentWaveState == EWaveState::GameOver) {
		GameOver();
		return;
	}

	CheckWaveState();
}

void ASGameMode::SpawnBotTimerElasped() {
	SpawnNewBot();

	numOfBotsToSpawn--;

	if (numOfBotsToSpawn <= 0) {
		EndWave();
	}
}