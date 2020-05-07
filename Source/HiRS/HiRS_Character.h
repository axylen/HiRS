// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HiRS_GmOptions.h"
#include "PoseSnapshot.h"

#include "HiRS_Character.generated.h"

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	idle		UMETA(DisplayName = "Idle"),
	move		UMETA(DisplayName = "Move"),
	sprint		UMETA(DisplayName = "Sprint"),
	TackleLT	UMETA(DisplayName = "Tackle Left"),
	TackleFD	UMETA(DisplayName = "Tackle Forward"),
	TackleRT	UMETA(DisplayName = "Tackle Right")
};

USTRUCT(BlueprintType)
struct FLagPose
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
		FVector Location;
	
	UPROPERTY(BlueprintReadOnly)
		FPoseSnapshot Snapshot;

	UPROPERTY(BlueprintReadOnly)
		FRotator Rotation;

	UPROPERTY(BlueprintReadOnly)
		float Time = -1.0f;
};

struct FLagPosesArray
{
		int CurrentID = 0;
		TArray<FLagPose> Poses;
};

UCLASS()
class HIRS_API AHiRS_Character : public ACharacter
{
	GENERATED_BODY()

private:
	float mvForwardVelocity = 0;
	float mvRightVelocity = 0;

	int JumpCount = 0;

	FLagPosesArray Poses;
	void AddPose();
	FTimerHandle PosesSnapHandle;

	void interpMovementSpeed();
	void CalculateMovementDirection();
	float GetSpeedNormalized();

	UFUNCTION()
	void OnRep_isDead();
	void SetIsDead(bool NewIsDead);

	const class AHiRS_PlayerController* HirsPC;
	const float ShootDelayAfterSprint = 0.2f;

public:
	// Sets default values for this character's properties
	AHiRS_Character();
	FVector OldCameraForwardVector;

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DynamicOutputParam = "Characters"))
		static void GetAllCharacters(const UObject* WorldContextObject, TArray<AHiRS_Character*>& Characters);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Landed(const FHitResult& Hit) override;

	UFUNCTION()
	void OnAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	UFUNCTION()
	void OnPointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDeath"))
	void EventOnDeath();

	void MoveSprint();
	bool GetCanSprint();

public:
	void TraceFromCamera();
	void TraceFromCamera(FHitResult& Hit, float Distance, bool bIsRewindedShot = false, FVector CharacterShift = FVector::ZeroVector);
	FVector GetMeshLocation();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/*
	* Character Desired Rotation
	*/
	UFUNCTION(BlueprintCallable, Category = "HIRS|Movement")
		virtual void CharacterDesiredRotation(float startRot = 85.0f, float RotationSpeed = 180.0f);

	/*
	* Update Character delta rotation
	* @return new delta rotation
	*/
	UFUNCTION(BlueprintCallable, Category = "HIRS|Movement")
		FRotator UpdateCharacterDeltaRotation();

	
	UFUNCTION(BlueprintCallable)
		void SetBoneScale(FVector Pelvis = FVector(1.0), FVector Spine1 = FVector(1.0), FVector Spine2 = FVector(1.0), FVector Spine3 = FVector(1.0), FVector Head = FVector(1.0), FVector RightLeg = FVector(1.0), FVector LeftLeg = FVector(1.0));

	UFUNCTION(BlueprintCallable)
		void RewindMesh(float atTime);

protected:
	void MoveFB(float inputAxis);
	void MoveRL(float inputAxis);

	void TurnPitch(float inputAxis);
	void TurnYaw(float inputAxis);

	void SprintPressed();
	void SprintReleased();
	bool isSprintBTNPressed;
	/**Last time of Sprint*/
	UPROPERTY(Transient, Replicated)
	float LastSprintTime;

	void JumpPressed();
	void JumpReleased();

	void TacklePressed();
	/**Last time of Tackle*/
	UPROPERTY(Transient, Replicated)
		float LastTackleTime;

	void FirePressed();
	void FireReleased();
	bool isFireBTNPressed;

	void SpecialWeaponFire();


public:
	/*
	* Set do Character want to Sprint or not
	* @param bNewRunnig Want to run or not
	*/
	UFUNCTION(BlueprintCallable)
		void SetWantToRun(bool bNewRunning);
	UFUNCTION(reliable, server, WithValidation, Category = "HIRS|Movement")
		void ServerSetWantToRun(bool bNewRunning);

		void SetRunning(bool bNewRunning);
	UFUNCTION(reliable, server, WithValidation)
		void ServerSetRunning(bool bNewRunning);

	/*
	* Set do Character aiming or not
	* @param bNewAiming	Do character want to aim
	*/
	UFUNCTION(BlueprintCallable)
		void SetAiming(bool bNewAiming);
	UFUNCTION(reliable, server, WithValidation, Category = "HIRS|Movement")
		void ServerSetAiming(bool bNewAiming);

	UFUNCTION(BlueprintCallable)
		void SetVelocity(FVector Direction, float power = 500.0f, bool Add = true);
	UFUNCTION(reliable, server, WithValidation, Category = "HIRS|Movement")
		void ServerSetVelocity(FVector Direction, float power = 1000.0f, bool Add = true);

	UFUNCTION(BlueprintCallable)
		void SetJumpZVelocity(float Velocity);
	UFUNCTION(reliable, server, WithValidation, Category = "HIRS|Movement")
		void ServerSetJumpZVelocity(float Velocity);

	UFUNCTION(BlueprintCallable)
		bool HirsTakeDamage(float damage = 0);


	UFUNCTION(BlueprintImplementableEvent)
		void OnTackle();

protected:
	/** Last sprint time */
	UFUNCTION(BlueprintCallable, Category = "HIRS|Movement")
		float GetLastSprintTime() const;


public:
	/*
	* Character Forward & Right movement input direction
	* @return movement input direction
	*/
	UFUNCTION(BlueprintPure, Category = "HIRS|Movement")
		FVector GetCharacterMovementInputDirection();

	/*
	* Camera forward vector
	* @return Camera forward vector
	*/
	UFUNCTION(BlueprintCallable, Category = "HIRS")
		FVector GetCameraForwardVector() const;

	/*
	* Camera world location
	* @return Camera forward vector
	*/
	UFUNCTION(BlueprintCallable, Category = "HIRS")
		FVector GetCameraWorldLocation() const;

	/*
	* Character pose at Time
	* @return [0,1] alpha for blend
	*/
	UFUNCTION(BlueprintCallable, Category = "HIRS")
		float GetPoseAtTime(float Time, FLagPose& PrevPose, FLagPose& NextPose);

	/*
	* Current ping of the player
	* @return actual ping
	*/
	UFUNCTION(BlueprintCallable, Category = "HIRS")
		int GetPing() const;

	UFUNCTION(BlueprintCallable)
		class AHIRS_PlayerState* GetHirsPlayerState() const;

	UFUNCTION(BlueprintCallable)
		ECurrentTeam GetCurrentTeam() const;

	UFUNCTION(BlueprintCallable)
		bool isInTheSameTeam(AActor* ActorForCheck) const;

	UFUNCTION(BlueprintCallable)
		bool CanBeDamagedBy(AActor* ActorForCheck) const;

protected:
	/** Character input interpolation speed */
	UPROPERTY(EditDefaultsOnly, Category = "HIRS|Movement")
		float inputInterpolationSpeed = 10.0f;

	/** Character Jog Max Speed */
	UPROPERTY(EditDefaultsOnly, Category = "HIRS|Movement")
		float JogSpeed = 400.0f;

	/** Character Sprint Max Speed */
	UPROPERTY(EditDefaultsOnly, Category = "HIRS|Movement")
		float SprintSpeed = 640.0f;

	/** Character Aiming Movement Max Speed */
	UPROPERTY(EditDefaultsOnly, Category = "HIRS|Movement")
		float AimMovementSpeed = 250.0f;

	/** Character Aiming Movement Speed */
	UPROPERTY(EditDefaultsOnly, Category = "HIRS|Movement")
		float MovementInterpolationSpeed = 10.0f;

	/** Characters health */
	UPROPERTY(Transient, replicated, BlueprintReadWrite, EditDefaultsOnly, Category = "HIRS")
		float Health = 200;

	/** Is allowed to move Left and Right while character Sprinting */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "HIRS|Movement")
		bool bAllowMoveSideWhileSprinting;

	/** Count of pose snapshots per second */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "HIRS|Network")
		int LagCompensationSnapCountPerSecond = 16;

	/** Character Ground jump distance */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "HIRS|Movement|Jump")
		float GroundJumpDistance = 0.0f;
	/** Character Air jump distance */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "HIRS|Movement|Jump")
		float AirJumpDistance = 1000.0f;
	/** Character Ground jump zVelocity */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "HIRS|Movement|Jump")
		float GroundJumpZVelocity = 500.0f;
	/** Character Air jump zVelocity */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "HIRS|Movement|Jump")
		float AirJumpZVelocity = 1000.0f;
	/** How much velocity character will save after airjump */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "HIRS|Movement|Jump")
		float SaveVelocity = 0.25f;
	/** Delay after tackle */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "HIRS|Movement")
		float TackleDelay = 3.0f;

public:
	/** Delay after tackle */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		float HeadshotDamageMultiplier = 4.0f;

public:
	/** Is Character aiming */
	UPROPERTY(Transient, Replicated, BlueprintReadOnly, Category = "HIRS|Character")
		bool bIsAiming = false;

	/** Is Character Want To Sprint */
	UPROPERTY(Transient, Replicated, BlueprintReadOnly, Category = "HIRS|Movement")
		bool bWantToSprint;

	/** Is Character Sprinting */
	UPROPERTY(Transient, Replicated, BlueprintReadOnly, Category = "HIRS|Movement")
		bool bIsSprinting;

	/** Is Character Tackle */
	UPROPERTY(Transient, Replicated, BlueprintReadWrite, Category = "HIRS|Movement")
		bool bIsTackle;

	/** Character delta rotation */
	UPROPERTY(Transient, Replicated, BlueprintReadOnly, Category = "HIRS|Movement")
		FRotator ControlRotation;

	/** Weapon Reference */
	UPROPERTY(Transient, replicated, BlueprintReadWrite, Category = "HIRS|Weapon")
		class AHiRS_Weapon* WeaponRef;

	/** Character camera view point */
	UPROPERTY(Transient, replicated, BlueprintReadOnly, Category = "HIRS")
		FVector CameraViewPoint = FVector(100);

	UPROPERTY(Transient, Replicated, BlueprintReadWrite)
		ECharacterState CharacterState;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_isDead, BlueprintReadWrite)
		bool isDead;
	
public:
	/** Is Character rotating */
	UPROPERTY(BlueprintReadWrite, Category = "HIRS|Movement")
		bool bIsNeedToRotate;

	/** Character delta rotation */
	UPROPERTY(BlueprintReadOnly, Category = "HIRS|Movement")
		FRotator DeltaRotation;

	/** Character Movement Direction in [-1,1] space */
	UPROPERTY(BlueprintReadOnly, Category = "HIRS|Movement")
		FVector MovementDirection;

	/** Character Movement Direction in [-180,180] space */
	UPROPERTY(BlueprintReadOnly, Category = "HIRS|Movement", meta = (DisplayName = "MovementDirection"))
		float MovementDirectionAngle;

	UPROPERTY(BlueprintReadOnly)
		float SpeedNormalized;

public:
	/**Spring Arm Component*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		class USpringArmComponent* SpringArm;

	/**Mesh for Momental Shot lag compensation*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		class USkeletalMeshComponent* MeshForCompensation;

	/**Camera Collision*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		class USphereComponent* CameraCollision;

	/**Actual camera*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		class UCameraComponent* Camera;

};
