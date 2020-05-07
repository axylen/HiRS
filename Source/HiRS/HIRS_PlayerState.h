// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "HiRS_GmOptions.h"
#include "HiRS.h"

#include "HIRS_PlayerState.generated.h"

/**
 * 
 */
UCLASS()
class HIRS_API AHIRS_PlayerState : public APlayerState
{
	GENERATED_BODY()


public:

	AHIRS_PlayerState();



	/* Increase kills count */
	UFUNCTION(BlueprintCallable)
		void AddKill();

	/* Increase deaths count */
	UFUNCTION(BlueprintCallable)
		void AddDeath();



	/* Get count of kills */
	UFUNCTION(BlueprintPure)
		int GetKills();

	/* Get count of deaths */
	UFUNCTION(BlueprintPure)
		int GetDeaths();

	/* Get KD Ratio*/
	UFUNCTION(BlueprintPure)
		float GetKDRatio();




		/* Replicated*/
	UPROPERTY(Transient, Replicated)
		uint16 Kills;

	UPROPERTY(Transient, Replicated)
		uint16 Deaths;
	
	/* Current Team*/
	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
		ECurrentTeam Team;

};
