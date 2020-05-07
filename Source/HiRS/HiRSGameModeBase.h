// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "HiRS_GmOptions.h"
#include "HiRS.h"

#include "HiRSGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class HIRS_API AHiRSGameModeBase : public AGameMode
{
	GENERATED_BODY()

public:

	AHiRSGameModeBase();
	virtual void PostLogin(APlayerController* NewPlayer);
	virtual void SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC);


	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayerConnected(AHiRS_PlayerController* NewPlayer);



	bool isFriendlyFireEnabled();

	UPROPERTY(BlueprintReadWrite)
	TArray<AHiRS_PlayerController*> AllPlayerControllers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = GMSetting)
	EGamemode CurrentGamemode;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = GMSetting)
	bool bEnableFriendlyFire;
};
