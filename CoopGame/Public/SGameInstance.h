// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "SWeapon.h"
#include "Engine/GameInstance.h"
#include "SGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API USGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FText PlayerName;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> PlayerMainWeaponClass;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> PlayerSecondaryWeaponClass;

	UPROPERTY(BlueprintReadWrite)
	int PlayerTeam;
};
