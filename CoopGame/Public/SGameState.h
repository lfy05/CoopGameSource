// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Sound/SoundCue.h"
#include "SGameState.generated.h"

UENUM(BlueprintType)
enum class EWaveState: uint8 {

	WaitingToStart,

	WaveInProgress,

	// no longer spawning new bots, waiting for players to kill remaining bots
	WaitingToComplete,

	WaveComplete,

	GameOver
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveCountUpdated, int32, NewWaveCount);

/**
 * 
 */
UCLASS()
class COOPGAME_API ASGameState : public AGameStateBase
{
	GENERATED_BODY()

protected:
	UFUNCTION()
	void OnRep_WaveState(EWaveState OldState);

	UFUNCTION(BlueprintImplementableEvent, Category = "GameState")
	void WaveStateChanged(EWaveState NewState, EWaveState OldState);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState, Category = "GameState")
	EWaveState WaveState;

	UPROPERTY(ReplicatedUsing = OnRep_WaveCount)
	int32 WaveCount;

	UFUNCTION()
	void OnRep_WaveCount();

	UPROPERTY(BlueprintAssignable, Category = "GameState")
	FOnWaveCountUpdated OnWaveCountUpdated;

public:
	void SetWaveState(EWaveState NewState);

	EWaveState GetWaveState();

	int32 GetWaveCount();

	void SetWaveCount(int32 NewWaveCount);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const;

	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	USoundCue *WaveStartSFX;

	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	USoundCue *GameOverSFX;

	UFUNCTION(NetMulticast, Reliable)
	void PlayWaveStartSFX();

	UFUNCTION(NetMulticast, Reliable)
	void PlayGameOverSFX();

};
