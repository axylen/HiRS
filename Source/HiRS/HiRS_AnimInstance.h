// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HiRS_AnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class HIRS_API UHiRS_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="HiRS|Character")
		FVector PelvisScale = {1.0, 1.0, 1.0};
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "HiRS|Character")
		FVector Spine1Scale = { 1.0, 1.0, 1.0 };
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "HiRS|Character")
		FVector Spine2Scale = { 1.0, 1.0, 1.0 };
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "HiRS|Character")
		FVector Spine3Scale = { 1.0, 1.0, 1.0 };
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "HiRS|Character")
		FVector HeadScale = { 1.0, 1.0, 1.0 };
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "HiRS|Character")
		FVector RightLegScale = { 1.0, 1.0, 1.0 };
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "HiRS|Character")
		FVector LeftLegScale = { 1.0, 1.0, 1.0 };

	UPROPERTY(BlueprintReadOnly, Category = "HiRS|Character")
		FPoseSnapshot PrevSnap;
	UPROPERTY(BlueprintReadOnly, Category = "HiRS|Character")
		FPoseSnapshot NextSnap;
	UPROPERTY(BlueprintReadOnly, Category = "HiRS|Character")
		float SnapBlendAlpha;


	UFUNCTION(BlueprintCallable)
		void SetSnapParams(FPoseSnapshot prev, FPoseSnapshot next, float alpha);
};
