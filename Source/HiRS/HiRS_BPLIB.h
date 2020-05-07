// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "HiRS_BPLIB.generated.h"


UCLASS()
class HIRS_API UHiRS_BPLIB : public UObject
{
	GENERATED_BODY()

public:
	/**
	* Return shortest angle between two in [-180, 180] range
	* @param from	start angle
	* @param to		end angle
	* @return		Shortest angle
	*/
	UFUNCTION(BlueprintPure)
		static float GetShortestAngle(float from = 0, float to = 360);

	/**
	* Same as FInterpTo, but change current value
	* @param current	current float value
	* @param target		target float value
	* @param delta		delta seconds
	* @param speed		interpolation speed
	* @param Difference Difference between old and new Current value
	* @return			New current value
	*/
	UFUNCTION(BlueprintCallable)
		static float FInterp(UPARAM(ref) float& current, float target, float delta, float& Difference, float speed = 10);

	/**
	* Distance between two points
	* @param from	start location
	* @param to		end location
	* @return			Distance
	*/
	UFUNCTION(BlueprintPure, Category = "HIRS Lib")
		static float VDistance(FVector from, FVector to);

};
