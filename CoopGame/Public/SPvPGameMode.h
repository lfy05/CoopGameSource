// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SPvPGameMode.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASPvPGameMode : public AGameMode
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Game Parameters")
	int TeamSize;

public:
	ASPvPGameMode();
	
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
};
