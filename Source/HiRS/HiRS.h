// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "HiRS_PlayerController.h"

#define ECC_Shot ECC_GameTraceChannel1
#define ECC_RewindShot ECC_GameTraceChannel2
#define ECC_Projectile ECC_GameTraceChannel3
#define ECC_World ECC_GameTraceChannel4


#define PString(time, string) if(GEngine) {GEngine->AddOnScreenDebugMessage(-1, time, FColor::Yellow, string);}

UENUM(BlueprintType)
enum class EAuthority : uint8
{
	localClient		UMETA(DisplayName = "Local Client"),
	otherClient		UMETA(DisplayName = "Other Client"),
	localServer		UMETA(DisplayName = "Local Server"),
	server			UMETA(DisplayName = "Server"),
	dedicatedServer	UMETA(DisplayName = "Dedicated Server")
};
