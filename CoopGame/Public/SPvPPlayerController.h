// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SPvPPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASPvPPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	int TeamNum = -1;
	
	UFUNCTION(BlueprintCallable, Category = "PVP")
	void UpdateDisplayName(FString NewDisplayName);
};
