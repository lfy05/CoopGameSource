// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPvPPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASPvPPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	int TeamNum;

};
