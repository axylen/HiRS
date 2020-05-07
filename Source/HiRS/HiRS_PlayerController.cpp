// Fill out your copyright notice in the Description page of Project Settings.

#include "HiRS_PlayerController.h"
#include "HiRS.h"
#include "HIRS_PlayerState.h"



AHiRS_PlayerController::AHiRS_PlayerController()
{
	
}


void AHiRS_PlayerController::SetSensetivity(float newSens)
{
	if (newSens > 0)
	{
		Sensetivity = newSens;
	}
}

int AHiRS_PlayerController::GetPing() const
{
	if (!this || !PlayerState) return 0;
	
	return PlayerState->Ping * 4;
}

ECurrentTeam AHiRS_PlayerController::GetCurrentTeam() const
{
	if (AHIRS_PlayerState* HPlayerState = GetPlayerState())
	{
		return HPlayerState->Team;
	}

	return ECurrentTeam::CT_Undefined;
}

AHIRS_PlayerState * AHiRS_PlayerController::GetPlayerState() const
{
	return Cast<AHIRS_PlayerState>(PlayerState);
}

void AHiRS_PlayerController::DebugOn()
{
	ConsoleCommand("t.MaxFPS = 999");
	ConsoleCommand("Stat UNIT");
	ConsoleCommand("Stat FPS");
}

void AHiRS_PlayerController::SetTimeSeconds(float NewTime)
{
	GetWorld()->TimeSeconds = NewTime;
}
