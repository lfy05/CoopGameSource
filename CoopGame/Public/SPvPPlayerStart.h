// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "SPvPPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASPvPPlayerStart : public APlayerStart
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditInstanceOnly, Category = "PVP")
	int TeamNumber;

public:
	ASPvPPlayerStart(const FObjectInitializer& ObjectInitializer);
};
