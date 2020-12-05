// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdated, int, NewScore);
/**
 * 
 */
UCLASS()
class COOPGAME_API ASPlayerState : public APlayerState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable)
	FOnScoreUpdated OnScoreUpdated;
	
public:
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "PlayerState")
	void AddScore(float ScoreDelta);
};
