// Fill out your copyright notice in the Description page of Project Settings.

#include "HiRS_AnimInstance.h"

void UHiRS_AnimInstance::SetSnapParams(FPoseSnapshot prev, FPoseSnapshot next, float alpha)
{
	PrevSnap = prev;
	NextSnap = next;
	SnapBlendAlpha = alpha;
}
