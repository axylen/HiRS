// Fill out your copyright notice in the Description page of Project Settings.

#include "HiRS_BPLIB.h"
#include "HiRS.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "HiRS_Character.h"

float UHiRS_BPLIB::GetShortestAngle(float from, float to)
{
	float temp = (to - from) / 360.0f;
	temp -= floor(temp);
	temp += (540.0f / 360.0f);
	temp -= floor(temp);
	return temp * 360 - 180;
}

float UHiRS_BPLIB::FInterp(UPARAM(ref) float& current, float target, float delta, float& Difference, float speed)
{
	float old = current;
	current = FMath::FInterpTo(current, target, delta, speed);
	Difference = current - old;
	return current;
}

float UHiRS_BPLIB::VDistance(FVector from, FVector to)
{
	return (to - from).Size();
}
