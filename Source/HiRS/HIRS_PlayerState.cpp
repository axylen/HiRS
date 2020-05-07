// Fill out your copyright notice in the Description page of Project Settings.

#include "HIRS_PlayerState.h"


AHIRS_PlayerState::AHIRS_PlayerState()
{
	Team = ECurrentTeam::CT_Undefined;
}


void AHIRS_PlayerState::AddKill()
{
	Kills++;
}

void AHIRS_PlayerState::AddDeath()
{
	Deaths++;

}



int AHIRS_PlayerState::GetKills()
{
	return Kills;
}

int AHIRS_PlayerState::GetDeaths()
{
	return Deaths;
}

float AHIRS_PlayerState::GetKDRatio()
{
	if (Deaths)
	{
		return (float)Kills / (float)Deaths;
	}
	return Kills;
}


/******************** LIFETIME REPLICATED PROPS */
void AHIRS_PlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHIRS_PlayerState, Kills);
	DOREPLIFETIME(AHIRS_PlayerState, Deaths);
	DOREPLIFETIME(AHIRS_PlayerState, Team);

}
