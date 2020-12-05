// Fill out your copyright notice in the Description page of Project Settings.
// game mode only exists on the server
// replicated information is put into game state and player state

#pragma once

#include "SGameState.h"
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Sound/SoundCue.h"
#include "SGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor *, VictimActor, AActor *, KillerActor, AController *, KillerController);
// 

/**
 * 
 */
UCLASS()
class COOPGAME_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	FTimerHandle TimerHandle_BotSpawner;
	FTimerHandle TimerHandle_NextWaveStarts;

	// number of bots to spawn in the current wave
	int32 numOfBotsToSpawn;

	// int32 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode");
	float TimeBetweenWaves;

	// Hook BP to spawn a single bot
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();

	void SpawnBotTimerElasped();

	// start spawning bots
	void StartWave();

	// stop spawning bots
	void EndWave();

	// set timer for next spawn wave
	void PrepareForNextWave();

	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

	void SetWaveState(EWaveState NewState);

	void RestartDeadPlayers();

public:

	ASGameMode();

	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnActorKilled OnActorKilled;

	//UPROPERTY(BlueprintAssignable, Category = "GameMode")
	//FOnNewWaveStarted OnNewWaveStarted;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode", meta = (ClampMin = "1"))
	int SpawnRateMultiplier;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode", meta = (ClampMin = "0"))
	int SpawnRateAdder;
};
