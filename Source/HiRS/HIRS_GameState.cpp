// Fill out your copyright notice in the Description page of Project Settings.

#include "HIRS_GameState.h"
#include "HiRS.h"
#include "HiRSGameModeBase.h"


void AHIRS_GameState::BeginPlay()
{
	if (Role == ROLE_Authority)
	{
		if (AHiRSGameModeBase* Gamemode = Cast<AHiRSGameModeBase>(GetWorld()->GetAuthGameMode()))
		{
			CurrentGamemode = Gamemode->CurrentGamemode;
			isFriendlyFireEnabled = Gamemode->bEnableFriendlyFire;
		}
	}
}

void AHIRS_GameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHIRS_GameState, CurrentGamemode);
	DOREPLIFETIME(AHIRS_GameState, isFriendlyFireEnabled);
}
