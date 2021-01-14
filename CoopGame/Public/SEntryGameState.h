// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SEntryGameState.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASEntryGameState : public AGameState
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void TravelToWorld();
	
};
