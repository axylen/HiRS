// Fill out your copyright notice in the Description page of Project Settings.

#include "HiRSGameModeBase.h"


AHiRSGameModeBase::AHiRSGameModeBase()
{
	CurrentGamemode = EGamemode::GM_FFA;
}

void AHiRSGameModeBase::PostLogin(APlayerController * NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AHiRS_PlayerController* HiRSPC = Cast<AHiRS_PlayerController>(NewPlayer);

	AllPlayerControllers.AddUnique(HiRSPC);
	OnPlayerConnected(HiRSPC);
}

void AHiRSGameModeBase::SwapPlayerControllers(APlayerController * OldPC, APlayerController * NewPC)
{
	Super::SwapPlayerControllers(OldPC, NewPC);

	AHiRS_PlayerController* HiRSPC = Cast<AHiRS_PlayerController>(NewPC);

	AllPlayerControllers.AddUnique(HiRSPC);
	OnPlayerConnected(HiRSPC);
}

bool AHiRSGameModeBase::isFriendlyFireEnabled()
{
	if (CurrentGamemode < EGamemode::GM_TDM || bEnableFriendlyFire)
	{
		return true;
	}

	return false;
}
