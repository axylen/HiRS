// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HiRS.h"
#include "WeaponsData.h"
#include "HiRS_Weapon.generated.h"

UCLASS()
class HIRS_API AHiRS_Weapon : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AHiRS_Weapon();

	EAuthority GetAuthority();
	AHiRS_PlayerController* GetController();
	class AHiRS_Character* GetHirsOwner();
	float GetLastShotTime();

	float RewindTime(FVector &CharacterShift);
	void MomentalDamage(FMomentalWeaponData& WeaponData, FHitResult& Hit);
	class AHiRS_Projectile* FireProjectile(const FProjectileWeaponData & WeaponData);
	void initSetup();
	bool isFriendlyFireEnabled;
	void addIgnoredActors(FCollisionQueryParams& CQParams);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	float CalculateWeaponShotDelay(float minDelay, float maxDelay, int StartChangeSpeed = 10, int ShotsTillMaxDelay = 40);
	void AddShotsCount(int MaxShotsInTheRow = 50);
	void DecreaseShotsInTheRowCount();


	/**
	* Shooting functions for Regular Weapon
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "HIRS|Weapon|Shooting")
		void SetupShootingFunctionsForWeapon();

	/**
	* Shooting functions for Special Weapon
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "HIRS|Weapon|Shooting")
		void SetupShootingFunctionsForSpecial();

public:	

	//*********************************************SERVER*********************************************

	void SetWantToShootWeapon(bool bNewWantToShoot);
	UFUNCTION(server, reliable, WithValidation)
		void ServerSetWantToShootWeapon(bool newWantToShoot);

	void UseSpecialWeapon();
	UFUNCTION(server, reliable, WithValidation)
		void ServerUseSpecialWeapon();

	/**
	* Change current used weapon
	* @param	Weapon			What weapon want to use
	*/
	UFUNCTION(BlueprintCallable, Category = "HIRS|Weapon|Shooting")
		void ChangeCurrentWeapon(EUsedWeapon Weapon);
	UFUNCTION(server, reliable, WithValidation)
		void ServerChangeCurrentWeapon(EUsedWeapon Weapon);



	//*********************************************PROPERTIES*****************************************
	/**Weapon last shot*/
	UPROPERTY(Transient, Replicated, BlueprintReadWrite)
		float WeaponLastShotTime = 0.0f;

	/**Weapon last shot*/
	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
		uint8 ShotsInTheRow = 0;

	/**Gravity gun last shot*/
	UPROPERTY(Transient, Replicated, BlueprintReadWrite)
		float SpecialWeaponLastShotTime = 0.0f;

	/**Current weapon*/
	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
		EUsedWeapon CurrentUsedWeapon = EUsedWeapon::primary;
	
	/**Do character want to shoot*/
	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
		bool bWantToShoot = false;
	
	/**Devide ping by*/
	UPROPERTY(EditDefaultsOnly)
		float RewindDevide = 1800.0;

	/**Current Authority*/
	UPROPERTY(BlueprintReadOnly)
		EAuthority CurrentAuthority = EAuthority::localClient;

	/**Gamemode Ref*/
	UPROPERTY(BlueprintReadOnly)
		class AHiRSGameModeBase* Gamemode;

protected:
	USceneComponent* Scene;
	/** Handle for efficient management of WeaponFiring timer */
	FTimerHandle TimerHandle_WeaponFire;
	/** Handle for efficient management of GravityFire timer */
	FTimerHandle TimerHandle_GravityFire;
	/** Handle for decrease shots in the row count */
	FTimerHandle TimerHandle_ShotsInTheRow;

	/**Weapon mesh*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		UStaticMeshComponent* WeaponMesh;


protected:
	bool SphereTrace(FHitResult& OutHit, FCollisionQueryParams & CQParams, ECollisionChannel CollChanel, FVector TraceStart, FVector TraceEnd, float Radius = 10);
	bool WorldTrace(FHitResult& OutHit, FVector TraceStart, FVector TraceEnd);
	/*Weapon Momental Shot*/
	UFUNCTION(BlueprintCallable)
		void MomentalWeaponShot(UPARAM(ref) FMomentalWeaponData& WeaponData);
	
	/*Weapon Projectile Shot*/
	UFUNCTION(BlueprintCallable)
		void ProjectileWeaponShot(UPARAM(ref) FProjectileWeaponData& WeaponData);

	/*Special Weapon Shot*/
	UFUNCTION(BlueprintCallable)
		void SpecialWeaponShot(UPARAM(ref) FProjectileWeaponData& WeaponData);


protected:
	UFUNCTION(NetMulticast, unreliable)
		void MC_OnWeaponMomentalShot(FHitResult HitResult);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnMomentalShot"))
		void EventOnWeaponMomentalShot(FHitResult HitResult);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "DrawShit"))
		void EventDrawShit(FVector center);

	UFUNCTION(NetMulticast, unreliable)
		void MC_OnWeaponProjectileShot(const class AHiRS_Projectile * Projectile);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnProjectileShot"))
		void EventOnWeaponProjectileShot(const class AHiRS_Projectile * Projectile);
};
