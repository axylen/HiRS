// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HiRS_GmOptions.h"

#include "HiRS_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class HIRS_API AHiRS_PlayerController : public APlayerController
{
	GENERATED_BODY()


public:

	AHiRS_PlayerController();

	UFUNCTION(Exec)
		void SetSensetivity(float newSens);

	UFUNCTION(Exec)
		void DebugOn();



	/* Current ping of the player */
	UFUNCTION(BlueprintCallable, Category = "HIRS")
		int GetPing() const;

	/* Current team of the player */
	UFUNCTION(BlueprintCallable, Category = "HIRS")
		ECurrentTeam GetCurrentTeam() const;

	/* HiRS PlayerState */
	UFUNCTION(BlueprintCallable, Category = "HIRS")
		class AHIRS_PlayerState* GetPlayerState() const;



	UFUNCTION(BlueprintCallable)
		void SetTimeSeconds(float NewTime);

	/* Sensetivity */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = Options)
		float Sensetivity = 0.25;
	
	/* Current Gamemode */
	UPROPERTY(BlueprintReadWrite)
		EGamemode CurrentGamemode;
	
};
