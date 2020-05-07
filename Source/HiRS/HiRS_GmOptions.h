// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EGamemode : uint8
{
	GM_FFA		UMETA(DisplayName = "Free for All"),
	GM_TDM		UMETA(DisplayName = "Team Death Match")
};

UENUM(BlueprintType)
enum class ECurrentTeam : uint8
{
	CT_Undefined	UMETA(DisplayName = "Undefined Team"),
	CT_FirstTeam	UMETA(DisplayName = "Team One"),
	CT_SecondTeam	UMETA(DisplayName = "Team Two")
};
