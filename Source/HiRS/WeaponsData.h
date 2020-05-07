// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponsData.generated.h"

UENUM(BlueprintType)
enum class EUsedWeapon : uint8
{
	primary			UMETA(DisplayName = "Primary"),
	alternative		UMETA(DisplayName = "Alternative")
};

USTRUCT(BlueprintType)
struct HIRS_API FProjectileWeaponData
{
	GENERATED_USTRUCT_BODY()

	/**Time between shots*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponData)
		float ShootingSpeed = 1.0f;

	/**Max time between shots*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponData)
		float MinShootingSpeed = 1.0f;

	/**How much shots need to start change shooting speed*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponData)
		int StartChangeSpeed = 10;

	/**How much shots need for MinShootingSpeed*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponData)
		int ShotsTillMaxDelay = 40;

	/**Is Weapon Automatic*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponData)
		bool bIsAutomatic = false;

	/**Projectile class to spawn*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponData)
		TSubclassOf<class AHiRS_Projectile> ProjectileClass;
	
};

USTRUCT(BlueprintType)
struct HIRS_API FMomentalWeaponData
{
	GENERATED_USTRUCT_BODY()

	/**Time between shots*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponData)
		float ShootingSpeed = 0.2f;

	/**Max time between shots*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponData)
		float MinShootingSpeed = 0.5f;

	/**How much shots need to start change shooting speed*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponData)
		int StartChangeSpeed = 10;

	/**How much shots need for MinShootingSpeed*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponData)
		int ShotsTillMaxDelay = 40;

	/**Is Weapon Automatic*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponData)
		bool bIsAutomatic = true;

	/**Default Damage*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponData)
		float Damage = 15.0f;

	/**Max shot distance*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponData)
		float MaxShotDistance = 10000.0f;

};
