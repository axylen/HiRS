// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "HiRS_GmOptions.h"

#include "HIRS_GameState.generated.h"

/**
 * 
 */
UCLASS()
class HIRS_API AHIRS_GameState : public AGameState
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(Transient, Replicated, BlueprintReadWrite)
		EGamemode CurrentGamemode;
	
	UPROPERTY(Transient, Replicated, BlueprintReadWrite)
		bool isFriendlyFireEnabled;
};
