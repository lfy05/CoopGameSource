// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SEntryGameMode.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASEntryGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void TravelToWorld(FString InURL);
	
};
