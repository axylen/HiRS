// Fill out your copyright notice in the Description page of Project Settings.

#include "HiRS_Character.h"
#include "HiRS_PlayerController.h"
#include "HiRS_AnimInstance.h"
#include "HIRS_PlayerState.h"
#include "HiRS_Weapon.h"
#include "HIRS_GameState.h"
#include "HiRS.h"

#include "CoreNet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
AHiRS_Character::AHiRS_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->RelativeLocation = FVector{ 0, -5, -90 };
	GetMesh()->RelativeRotation = FRotator{ 0, 270, 0 };
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraAttachmentArm"));
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->RelativeRotation = FRotator{ 0,90, 0 };
	SpringArm->RelativeLocation = FVector{ 0, 0, 180 };
	SpringArm->TargetArmLength = 200;
	SpringArm->ProbeSize = 32;
	SpringArm->bUsePawnControlRotation = true;
	
	CameraCollision = CreateDefaultSubobject<USphereComponent>(TEXT("CameraCollision"));
	CameraCollision->SetupAttachment(SpringArm);
	CameraCollision->RelativeLocation = FVector{ 0, 10, 0 };
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->RelativeLocation = FVector{ 0, 10, 0 };
	
	MeshForCompensation = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh for LagCompensation"));
	MeshForCompensation->SetupAttachment(RootComponent);
	MeshForCompensation->SetHiddenInGame(true);
	MeshForCompensation->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	MeshForCompensation->SetCollisionResponseToChannel(ECC_RewindShot, ECollisionResponse::ECR_Block);
	MeshForCompensation->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	MeshForCompensation->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshForCompensation->RelativeLocation = FVector{ 150, 0, -90 };
	MeshForCompensation->RelativeRotation = FRotator{ 0, 270, 0 };
	MeshForCompensation->CastShadow = false;
	MeshForCompensation->SetEnableGravity(false);
	MeshForCompensation->bApplyImpulseOnDamage = false;
	MeshForCompensation->bIgnoreRadialForce = true;
	MeshForCompensation->bIgnoreRadialImpulse = true;
	
	OnTakeAnyDamage.AddDynamic(this, &AHiRS_Character::OnAnyDamage);
	OnTakePointDamage.AddDynamic(this, &AHiRS_Character::OnPointDamage);

	//FDetachmentTransformRules DetachRules = FDetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	//MeshForCompensation->DetachFromComponent(DetachRules);
}

void AHiRS_Character::GetAllCharacters(const UObject* WorldContextObject, TArray<AHiRS_Character*>& Characters)
{
	TArray<AActor*> CharacterActors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject, AHiRS_Character::StaticClass(), CharacterActors);
	Characters.Empty();
	const int16 NumOfCharacters = CharacterActors.Num();

	for (int i = 0; i < NumOfCharacters; ++i)
	{
		Characters.Add(Cast<AHiRS_Character>(CharacterActors[i]));
	}
}

// Called to bind functionality to input
void AHiRS_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AHiRS_Character::MoveFB);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHiRS_Character::MoveRL);

	PlayerInputComponent->BindAxis("TurnUp", this, &AHiRS_Character::TurnPitch);
	PlayerInputComponent->BindAxis("TurnRight", this, &AHiRS_Character::TurnYaw);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AHiRS_Character::SprintPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AHiRS_Character::SprintReleased);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AHiRS_Character::JumpPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AHiRS_Character::JumpReleased);

	PlayerInputComponent->BindAction("Tackle", IE_Pressed, this, &AHiRS_Character::TacklePressed);

	PlayerInputComponent->BindAction("FireWeapon", IE_Pressed, this, &AHiRS_Character::FirePressed);
	PlayerInputComponent->BindAction("FireWeapon", IE_Released, this, &AHiRS_Character::FireReleased);

	PlayerInputComponent->BindAction("FireSpecialWeapon", IE_Pressed, this, &AHiRS_Character::SpecialWeaponFire);
}
	
// Called when the game starts or when spawned
void AHiRS_Character::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		Poses.Poses.SetNum(LagCompensationSnapCountPerSecond);
		GetWorldTimerManager().SetTimer(PosesSnapHandle, this, &AHiRS_Character::AddPose, 1.0 / LagCompensationSnapCountPerSecond, true);
	}

}

void AHiRS_Character::Landed(const FHitResult & Hit)
{
	Super::Landed(Hit);

	JumpCount = 0;
}

void AHiRS_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (Role == ROLE_Authority)
	{
		if (Controller)
		{
			ControlRotation = Controller->GetControlRotation();
		}
		TraceFromCamera();
	}
	else if (IsLocallyControlled())
	{
		TraceFromCamera();
	}
	
	UpdateCharacterDeltaRotation();
	interpMovementSpeed();

	CharacterDesiredRotation();

	SpeedNormalized = GetSpeedNormalized();

	if (GetVelocity().Size() > 0.001)
	{
		CalculateMovementDirection();
	}
	else
	{
		MovementDirection.X = 0.0;
		MovementDirection.Y = 0.0;
		MovementDirection.Z = 0.0;
		MovementDirectionAngle = 0.0;
	}
	
}



void AHiRS_Character::AddPose()
{
	if (!this || isDead) return;
	USkeletalMeshComponent * Mesh = GetMesh();
	UWorld * World = GetWorld();
	if (!Mesh || !World) return;

	FLagPose &NewPose = Poses.Poses[Poses.CurrentID];
	NewPose.Location = Mesh->GetComponentLocation();
	NewPose.Rotation = Mesh->GetComponentRotation();
	NewPose.Time = World->TimeSeconds;
	Mesh->SnapshotPose(NewPose.Snapshot);

	if (Poses.CurrentID == LagCompensationSnapCountPerSecond - 1) Poses.CurrentID = 0;
	else Poses.CurrentID++;
}


void AHiRS_Character::MoveSprint()
{
	
	bool isShotDelayComplete = true;
	
	if (WeaponRef)
	{
		float LastShotTime = WeaponRef->GetLastShotTime();
		isShotDelayComplete = (LastShotTime + 0.4 < GetWorld()->TimeSeconds) || (LastShotTime == 0.0f);
	}
	
	if (bWantToSprint)
	{
		bool isSpeedEnough = SpeedNormalized > 0.8 && mvForwardVelocity > 0.4;
		
		bool isFalling = GetCharacterMovement()->IsFalling();
		if ( isSpeedEnough && isShotDelayComplete && !isFalling) SetRunning(true);
		else SetRunning(false);
	}
	else
	{
		SetRunning(false);
	}
}
void AHiRS_Character::interpMovementSpeed()
{
	float iTo;

	if (bIsAiming) iTo = AimMovementSpeed;
	else if (bIsSprinting) iTo = SprintSpeed;
	else iTo = JogSpeed;

	GetCharacterMovement()->MaxWalkSpeed = FMath::FInterpTo(GetCharacterMovement()->MaxWalkSpeed, iTo, GetWorld()->DeltaTimeSeconds, MovementInterpolationSpeed);
}
bool AHiRS_Character::GetCanSprint()
{
	if (WeaponRef)
	{
		bool WantToShoot = WeaponRef->bWantToShoot;
		float LastShotTime = WeaponRef->GetLastShotTime();

		bool ActionTimeComplete = (LastShotTime + 0.4 < GetWorld()->TimeSeconds) || LastShotTime == 0;

		return ActionTimeComplete && !WantToShoot && !bIsAiming;
	}

	return true;
}

void AHiRS_Character::TraceFromCamera()
{
	FCollisionQueryParams QParam;
	QParam.AddIgnoredActor(this);
	if (WeaponRef) QParam.AddIgnoredActor(WeaponRef);

	FVector StartLocation = GetCameraWorldLocation();
	FVector Direction = GetCameraForwardVector();
	FVector EndLocation = StartLocation + Direction * 10000;
	
	FHitResult CameraTrace;

	GWorld->LineTraceSingleByChannel(CameraTrace, StartLocation, EndLocation, ECC_Shot, QParam);
	
	CameraViewPoint = CameraTrace.bBlockingHit ? CameraTrace.ImpactPoint : CameraTrace.TraceEnd;
}

void AHiRS_Character::TraceFromCamera(FHitResult & Hit, float Distance, bool bIsRewindedShot, FVector CharacterShift)
{
	FCollisionQueryParams QParam;
	QParam.AddIgnoredActor(this);
	if (WeaponRef) QParam.AddIgnoredActor(WeaponRef);

	ECollisionChannel ColChannel = bIsRewindedShot ? ECC_RewindShot : ECC_Shot;
	FVector StartLocation = GetCameraWorldLocation();
	if (bIsRewindedShot) StartLocation -= CharacterShift;
	FVector Direction = GetCameraForwardVector();
	FVector EndLocation = StartLocation + Direction * Distance;

	GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ColChannel, QParam);
}

FVector AHiRS_Character::GetMeshLocation()
{
	USkeletalMeshComponent* Mesh = GetMesh();
	if (!Mesh) return GetActorLocation();
	else return Mesh->GetComponentLocation();
}

void AHiRS_Character::CharacterDesiredRotation(float startRot, float RotationSpeed)
{
	UCharacterMovementComponent* cMovement = GetCharacterMovement();
	float speed = GetVelocity().Size();

	if (cMovement->IsFalling())
	{
		bIsNeedToRotate = false;
		cMovement->RotationRate = FRotator{ 0,720,0 };
	}
	else
	{
		if (speed < 20)
		{
			if (bIsNeedToRotate)
			{
				if (fabs(DeltaRotation.Yaw) < 5)
				{
					bIsNeedToRotate = false;
					cMovement->RotationRate = FRotator{ 0,0,0 };
				}
				else if (fabs(DeltaRotation.Yaw) > startRot)
				{
					float mp = fabs(DeltaRotation.Yaw) / (startRot / 2);
					RotationSpeed *= FMath::Clamp(mp, 1.f, 3.f);
					cMovement->RotationRate = FRotator{ 0,RotationSpeed,0 };
				}
			}
			else
			{
				if (fabs(DeltaRotation.Yaw) >= startRot)
				{
					float mp = fabs(DeltaRotation.Yaw) / (startRot / 2);
					RotationSpeed *= FMath::Clamp(mp, 1.f, 3.f);
					cMovement->RotationRate = FRotator{ 0,RotationSpeed,0 };
					bIsNeedToRotate = true;
				}
				else
				{
					cMovement->RotationRate = FRotator{ 0,0,0 };
				}
			}
		}
		else
		{
			bIsNeedToRotate = false;
			float RSpeed = FMath::Lerp(360, 180, speed / SprintSpeed);
			cMovement->RotationRate = FRotator{ 0, RSpeed,0 };
		}
	}
}
FRotator AHiRS_Character::UpdateCharacterDeltaRotation()
{
	FRotator CRot = IsLocallyControlled() ? GetControlRotation() : ControlRotation;
	return DeltaRotation = (CRot - GetActorRotation()).GetNormalized();
}


void AHiRS_Character::MoveFB(float inputAxis)
{
	bool isFalling = GetMovementComponent()->IsFalling();
	
	mvForwardVelocity = isFalling ? MovementDirection.X : FMath::FInterpTo(mvForwardVelocity, inputAxis, GetWorld()->DeltaTimeSeconds, inputInterpolationSpeed);
	
	if (isFalling)
	{
		MoveSprint();

		if (fabs(inputAxis) > 0.001)
		{
			AddMovementInput(GetActorForwardVector(), inputAxis);
		}

		return;
	}

	if (fabs(mvForwardVelocity) > 0.001)
	{
		MoveSprint();

		AddMovementInput(GetActorForwardVector(), mvForwardVelocity);
	}
}
void AHiRS_Character::MoveRL(float inputAxis)
{
	bool isFalling = GetMovementComponent()->IsFalling();

	mvRightVelocity = isFalling ? MovementDirection.Y : FMath::FInterpTo(mvRightVelocity, inputAxis, GetWorld()->DeltaTimeSeconds, inputInterpolationSpeed);

	if (isFalling)
	{
		if (fabs(inputAxis) > 0.001)
		{
			AddMovementInput(GetActorRightVector(), inputAxis);
		}

		return;
	}

	if (fabs(mvRightVelocity) > 0.001)
	{
		if (!bIsSprinting || (bAllowMoveSideWhileSprinting && mvForwardVelocity > 0.8))
		{
			AddMovementInput(GetActorRightVector(), mvRightVelocity);
		}
	}
}

void AHiRS_Character::TurnPitch(float inputAxis)
{
	if (fabs(inputAxis) > 0.001)
	{
		HirsPC = GetController() ? Cast<AHiRS_PlayerController>(GetController()) : NULL;
		float sens = HirsPC ? HirsPC->Sensetivity : 1;
		AddControllerPitchInput(inputAxis * sens);
	}
}
void AHiRS_Character::TurnYaw(float inputAxis)
{
	if (fabs(inputAxis) > 0.001)
	{
		HirsPC = GetController() ? Cast<AHiRS_PlayerController>(GetController()) : NULL;
		float sens = HirsPC ? HirsPC->Sensetivity : 1;
		float FinalAxis = inputAxis * sens;

		if (bIsTackle)
		{
			float newRot = DeltaRotation.Yaw + FinalAxis;
			float LeftRot, RightRot;

			if (CharacterState == ECharacterState::TackleLT)
			{
				LeftRot = -115;
				RightRot = 25;
			}
			else if (CharacterState == ECharacterState::TackleFD)
			{
				LeftRot = -70;
				RightRot = 70;
			}
			else
			{
				LeftRot = -25;
				RightRot = 115;
			}

			if (newRot > RightRot || newRot < LeftRot)
			{
				return;
			}
		}


		AddControllerYawInput(FinalAxis);

	}
}


void AHiRS_Character::SprintPressed()
{
	isSprintBTNPressed = true;
	SetWantToRun(true);
}
void AHiRS_Character::SprintReleased()
{
	isSprintBTNPressed = false;
	SetWantToRun(false);
}

void AHiRS_Character::JumpPressed()
{
	UCharacterMovementComponent * MovementComponent = GetCharacterMovement();
	if (!MovementComponent || !CanJump()) return;

	if (JumpCount >= JumpMaxCount) return;

	FVector JumpDirectionVelocity = GetCharacterMovementInputDirection();

	if (bIsSprinting) SetRunning(false);

	if (MovementComponent->IsFalling())
	{
		SetJumpZVelocity(AirJumpZVelocity);
		
		SetVelocity(JumpDirectionVelocity, AirJumpDistance, false);

		Jump();

		if (JumpCount == 0) JumpCount = 2;
		else JumpCount++;
	}
	else
	{
		SetJumpZVelocity(GroundJumpZVelocity);

		SetVelocity(JumpDirectionVelocity, GroundJumpDistance);

		Jump();

		JumpCount++;
	}
}
void AHiRS_Character::JumpReleased()
{
	StopJumping();
}

void AHiRS_Character::TacklePressed()
{
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (!MovementComp) return;

	const float Speed = SpeedNormalized;

	if (MovementComp->IsFalling() || Speed < 0.6) return;

	if (LastTackleTime + TackleDelay <= GWorld->TimeSeconds || LastTackleTime == 0.0) {

		if (MovementDirectionAngle < -60 || MovementDirectionAngle > 60) return;

		if (MovementDirectionAngle >= -20 && MovementDirectionAngle <= 20)
		{
			CharacterState = ECharacterState::TackleFD;
			OnTackle();
		}
		else if (MovementDirectionAngle < -20)
		{
			CharacterState = ECharacterState::TackleLT;
			OnTackle();
		}
		else
		{
			CharacterState = ECharacterState::TackleRT;
			OnTackle();
		}

		LastTackleTime = GWorld->TimeSeconds;
	}
}

void AHiRS_Character::FirePressed()
{
	isFireBTNPressed = true;

	if (!WeaponRef) return;
	
	const float allowedShotTime = GetLastSprintTime() + ShootDelayAfterSprint;
	const float& WorldTime = GetWorld()->TimeSeconds;

	if (WorldTime < allowedShotTime)
	{
		SetWantToRun(false);
		const float ShotDelay = allowedShotTime - WorldTime;

		FTimerHandle handle;
		GetWorld()->GetTimerManager().SetTimer(handle, [this]()
		{
			if (isFireBTNPressed)
			{
				WeaponRef->SetWantToShootWeapon(true);
			}
		}, ShotDelay, false);
		
	}
	else
	{
		WeaponRef->SetWantToShootWeapon(true);
	}

}
void AHiRS_Character::FireReleased()
{
	isFireBTNPressed = false;

	if (!WeaponRef) return;

	WeaponRef->SetWantToShootWeapon(false);
}
void AHiRS_Character::SpecialWeaponFire()
{
	if (!WeaponRef) return;
	SetWantToRun(false);

	WeaponRef->UseSpecialWeapon();
}



void AHiRS_Character::SetAiming(bool bNewAiming)
{
	if (bNewAiming == bIsAiming) return;

	bIsAiming = bNewAiming;
	if (Role < ROLE_Authority)
	{
		ServerSetAiming(bNewAiming);
	}
}
bool AHiRS_Character::ServerSetAiming_Validate(bool bNewAiming)
{
	return true;
}
void AHiRS_Character::ServerSetAiming_Implementation(bool bNewAiming)
{
	SetAiming(bNewAiming);
}

void AHiRS_Character::SetRunning(bool bNewRunning)
{
	if (bNewRunning == bIsSprinting) return;
	if ( !((LastSprintTime + 0.4 < GetWorld()->TimeSeconds) || (LastSprintTime == 0.0f)) ) return;

	bIsSprinting = bNewRunning;

	if (!bNewRunning) LastSprintTime = GetWorld()->TimeSeconds;

	if (Role < ROLE_Authority)
	{
		ServerSetRunning(bNewRunning);
	}
}
bool AHiRS_Character::ServerSetRunning_Validate(bool bNewRunning)
{
	return true;
}
void AHiRS_Character::ServerSetRunning_Implementation(bool bNewRunning)
{
	SetRunning(bNewRunning);
}

void AHiRS_Character::SetWantToRun(bool bNewRunning)
{
	if (bWantToSprint == bNewRunning) return;

	bWantToSprint = bNewRunning;
	if (Role < ROLE_Authority)
	{
		ServerSetWantToRun(bNewRunning);
	}
}
bool AHiRS_Character::ServerSetWantToRun_Validate(bool bNewRunning)
{
	return true;
}
void AHiRS_Character::ServerSetWantToRun_Implementation(bool bNewRunning)
{
	SetWantToRun(bNewRunning);
}

void AHiRS_Character::SetVelocity(FVector Direction, float power, bool Add)
{
	FVector Vel = Direction.GetSafeNormal() * power;
	if (Add) GetCharacterMovement()->Velocity += Vel;
	else GetCharacterMovement()->Velocity = Vel + GetCharacterMovement()->Velocity * SaveVelocity;
	
	if (Role < ROLE_Authority)
	{
		ServerSetVelocity(Direction, power, Add);
	}
}
bool AHiRS_Character::ServerSetVelocity_Validate(FVector Direction, float power, bool Add)
{
	return true;
}
void AHiRS_Character::ServerSetVelocity_Implementation(FVector Direction, float power, bool Add)
{
	SetVelocity(Direction, power, Add);
}

void AHiRS_Character::SetJumpZVelocity(float Velocity)
{
	UCharacterMovementComponent* CharacterMovement = GetCharacterMovement();

	CharacterMovement->JumpZVelocity = Velocity;

	if (Role < ROLE_Authority)
	{
		ServerSetJumpZVelocity(Velocity);
	}
}
bool AHiRS_Character::ServerSetJumpZVelocity_Validate(float Velocity)
{
	return true;
}
void AHiRS_Character::ServerSetJumpZVelocity_Implementation(float Velocity)
{
	SetJumpZVelocity(Velocity);
}



void AHiRS_Character::SetBoneScale(FVector Pelvis, FVector Spine1, FVector Spine2, FVector Spine3, FVector Head, FVector RightLeg, FVector LeftLeg)
{
	UHiRS_AnimInstance* AnimIntstance = Cast<UHiRS_AnimInstance>(GetMesh()->GetAnimInstance());
	if (!AnimIntstance) return;

	AnimIntstance->PelvisScale = Pelvis;
	AnimIntstance->Spine1Scale = Spine1;
	AnimIntstance->Spine2Scale = Spine2;
	AnimIntstance->Spine3Scale = Spine3;
	AnimIntstance->HeadScale = Head;
	AnimIntstance->RightLegScale = RightLeg;
	AnimIntstance->LeftLegScale = LeftLeg;
}

void AHiRS_Character::RewindMesh(float atTime)
{
	if (!MeshForCompensation) return;
	if (isDead) return;

	FLagPose SnapPrev, SnapNext;
	float SnapAlpha = GetPoseAtTime(atTime, SnapPrev, SnapNext);

	UHiRS_AnimInstance* AnimInstance = Cast<UHiRS_AnimInstance>( MeshForCompensation->GetAnimInstance() );
	AnimInstance->PrevSnap = SnapPrev.Snapshot;
	AnimInstance->NextSnap = SnapNext.Snapshot;
	AnimInstance->SnapBlendAlpha = SnapAlpha;

	FVector Location = UKismetMathLibrary::VLerp(SnapPrev.Location, SnapNext.Location, SnapAlpha);
	FRotator Rotation = UKismetMathLibrary::RLerp(SnapPrev.Rotation, SnapNext.Rotation, SnapAlpha, false);

	MeshForCompensation->SetWorldLocationAndRotation(Location, Rotation);
}

void AHiRS_Character::CalculateMovementDirection()
{
	FVector ForwardVector = GetActorForwardVector();
	FVector RightVector = GetActorRightVector();
	FVector Velocity = GetVelocity();
	FVector NormalizedVel = Velocity.GetSafeNormal();
	ForwardVector.Z = RightVector.Z = NormalizedVel.Z = 0.f;

	MovementDirection.X = FVector::DotProduct(ForwardVector, NormalizedVel);
	MovementDirection.Y = FVector::DotProduct(RightVector, NormalizedVel);
	MovementDirection.Z = Velocity.Z;

	float ForwardDeltaDegree = FMath::RadiansToDegrees(FMath::Acos(MovementDirection.X));

	if (MovementDirection.Y < 0)
	{
		ForwardDeltaDegree *= -1;
	}

	MovementDirectionAngle = ForwardDeltaDegree;
}

void AHiRS_Character::OnAnyDamage(AActor * DamagedActor, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (isDead) return;

	if (HirsTakeDamage(Damage))
	{
		if (AHIRS_PlayerState* HirsPlayerState = GetHirsPlayerState())
		{
			HirsPlayerState->AddDeath();
		}
		if (InstigatedBy)
		{
			if (AHiRS_Character* DamageInstigatedBy = Cast<AHiRS_Character>(InstigatedBy->GetPawn()))
			{
				if (AHIRS_PlayerState* HirsPlayerState = DamageInstigatedBy->GetHirsPlayerState())
				{
					HirsPlayerState->AddKill();
				}
			}
		}
	}

}

void AHiRS_Character::OnPointDamage(AActor * DamagedActor, float Damage, AController * InstigatedBy, FVector HitLocation, UPrimitiveComponent * FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType * DamageType, AActor * DamageCauser)
{

}

bool AHiRS_Character::HirsTakeDamage(float damage)
{
	if (Role < ROLE_Authority) return isDead;

	Health -= damage;

	if (Health <= 0.0f)
	{
		Health = 0.0f;
		SetIsDead(true);
	}

	return isDead;
}

void AHiRS_Character::OnRep_isDead()
{
	if (isDead)
	{
		EventOnDeath();
	}
}

void AHiRS_Character::SetIsDead(bool NewIsDead)
{
	isDead = NewIsDead;

	if (isDead)
	{
		EventOnDeath();
	}
}


float AHiRS_Character::GetSpeedNormalized()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	return Velocity.Size() / JogSpeed;
}

FVector AHiRS_Character::GetCharacterMovementInputDirection()
{
	if (!InputComponent) return FVector::ZeroVector;
	return (GetActorForwardVector() * InputComponent->GetAxisValue("MoveForward")) + (GetActorRightVector() * InputComponent->GetAxisValue("MoveRight"));
}

FVector AHiRS_Character::GetCameraForwardVector() const
{
	if (Camera)
	{
		return IsLocallyControlled() ? Camera->GetForwardVector() : ControlRotation.Vector();
	}
	
	return FVector{ 0,0,0 };
}
FVector AHiRS_Character::GetCameraWorldLocation() const
{
	return Camera ? Camera->K2_GetComponentLocation() : FVector{ 0,0,0 };
}

float AHiRS_Character::GetLastSprintTime() const
{
	if (bIsSprinting)
	{
		UWorld* World = GetWorld();
		return World ? World->GetTimeSeconds() : LastSprintTime;
	}
	return LastSprintTime;
}

float AHiRS_Character::GetPoseAtTime(float Time, FLagPose& PrevPose, FLagPose& NextPose)
{
	for (int i = 0; i < LagCompensationSnapCountPerSecond; ++i)
	{

		if (i == LagCompensationSnapCountPerSecond - 1)
		{
			if (Poses.Poses[i].Time > Poses.Poses[0].Time)
			{
				PrevPose = Poses.Poses[i];

				if (USkeletalMeshComponent* Mesh = GetMesh())
				{
					FLagPose NewPose;
					NewPose.Location = Mesh->GetComponentLocation();
					NewPose.Rotation = Mesh->GetComponentRotation();
					NewPose.Time = GetWorld()->TimeSeconds;
					Mesh->SnapshotPose(NewPose.Snapshot);
					NextPose = NewPose;

					float delta = NextPose.Time - PrevPose.Time;
					return (Time - PrevPose.Time) / delta;
				}
				else
				{
					NextPose = Poses.Poses[i];

					return 0.0f;
				}
			}
			else
			{
				PrevPose = Poses.Poses[i];
				NextPose = Poses.Poses[0];

				float delta = NextPose.Time - PrevPose.Time;
				return (Time - PrevPose.Time) / delta;
			}
		}
		else
		{
			if (Poses.Poses[i].Time > Poses.Poses[i + 1].Time)
			{
				if (Poses.Poses[i].Time <= Time)
				{
					PrevPose = Poses.Poses[i];

					if (USkeletalMeshComponent* Mesh = GetMesh())
					{
						FLagPose NewPose;
						NewPose.Location = Mesh->GetComponentLocation();
						NewPose.Rotation = Mesh->GetComponentRotation();
						NewPose.Time = GetWorld()->TimeSeconds;
						Mesh->SnapshotPose(NewPose.Snapshot);
						NextPose = NewPose;

						float delta = NextPose.Time - PrevPose.Time;
						return (Time - PrevPose.Time) / delta;
					}
					else
					{
						NextPose = Poses.Poses[i];
						
						return 0.0f;
					}
				}
			}
			else
			{
				if (Poses.Poses[i].Time <= Time && Poses.Poses[i + 1].Time > Time)
				{
					PrevPose = Poses.Poses[i];
					NextPose = Poses.Poses[i + 1];

					float delta = NextPose.Time - PrevPose.Time;
					return (Time - PrevPose.Time) / delta;
				}
			}
		}

	}


	if (USkeletalMeshComponent* Mesh = GetMesh())
	{
		FLagPose NewPose;
		NewPose.Location = Mesh->GetComponentLocation();
		NewPose.Rotation = Mesh->GetComponentRotation();
		NewPose.Time = GetWorld()->TimeSeconds;
		Mesh->SnapshotPose(NewPose.Snapshot);
		NextPose = NewPose;
		PrevPose = NewPose;

		return 0.0;
	}
	return 0.0f;
}

int AHiRS_Character::GetPing() const
{
	if (!this || !PlayerState) return 0;

	return PlayerState->Ping * 4;
}

AHIRS_PlayerState * AHiRS_Character::GetHirsPlayerState() const
{
	return Cast<AHIRS_PlayerState>(PlayerState);
}

ECurrentTeam AHiRS_Character::GetCurrentTeam() const
{
	if (AHIRS_PlayerState* State = GetHirsPlayerState())
	{
		return State->Team;
	}

	return ECurrentTeam::CT_Undefined;
}

bool AHiRS_Character::isInTheSameTeam(AActor * ActorForCheck) const
{
	AHiRS_Character* HiRSActor = Cast<AHiRS_Character>(ActorForCheck);

	if (HiRSActor && HirsPC)
	{
		return GetCurrentTeam() == HiRSActor->GetCurrentTeam();
	}
	else
	{
		return false;
	}
}

bool AHiRS_Character::CanBeDamagedBy(AActor * ActorForCheck) const
{
	AHIRS_GameState* GameState = Cast<AHIRS_GameState>(GetWorld()->GetGameState());

	if (GameState->CurrentGamemode < EGamemode::GM_TDM || GameState->isFriendlyFireEnabled)
	{
		return true;
	}
	else
	{
		return !isInTheSameTeam(ActorForCheck);
	}
}


void AHiRS_Character::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHiRS_Character, bIsAiming);
	DOREPLIFETIME(AHiRS_Character, bIsSprinting);
	DOREPLIFETIME(AHiRS_Character, WeaponRef);
	DOREPLIFETIME(AHiRS_Character, isDead);
	
	DOREPLIFETIME_CONDITION(AHiRS_Character, ControlRotation, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHiRS_Character, CameraViewPoint, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHiRS_Character, CharacterState, COND_SkipOwner);
	
	DOREPLIFETIME_CONDITION(AHiRS_Character, bWantToSprint, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHiRS_Character, LastTackleTime, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHiRS_Character, LastSprintTime, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHiRS_Character, bIsTackle, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHiRS_Character, Health, COND_OwnerOnly);
}
