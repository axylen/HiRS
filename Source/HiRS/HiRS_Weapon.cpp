// Fill out your copyright notice in the Description page of Project Settings.

#include "HiRS_Weapon.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "HiRS_Projectile.h"
#include "HiRS_Character.h"
#include "HiRS_PlayerController.h"
#include "HIRS_GameState.h"
#include "HiRSGameModeBase.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/DamageType.h"




// Sets default values
AHiRS_Weapon::AHiRS_Weapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Component"));
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon"));

	RootComponent = Scene;
	WeaponMesh->SetupAttachment(RootComponent);

	bReplicates = true;
}


float AHiRS_Weapon::GetLastShotTime()
{
	return fmax(WeaponLastShotTime, SpecialWeaponLastShotTime);
}


// Called when the game starts or when spawned
void AHiRS_Weapon::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle handle;
	UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(handle, [this]()
	{
		CurrentAuthority = GetAuthority();
	}, 0.1, false);
	
	initSetup();
}


void AHiRS_Weapon::initSetup()
{
	Gamemode = Cast<AHiRSGameModeBase>(UGameplayStatics::GetGameMode(this));

	if (Gamemode)
	{
		isFriendlyFireEnabled = Gamemode->isFriendlyFireEnabled();
	}
}

void AHiRS_Weapon::addIgnoredActors(FCollisionQueryParams & CQParams)
{
	UWorld* World = GetWorld();
	AHIRS_GameState* GameState = Cast<AHIRS_GameState>(World->GetGameState());
	AHiRS_Character* HiRSOwner = GetHirsOwner();
	if (!HiRSOwner || !GameState) return;

	if (GameState->CurrentGamemode < EGamemode::GM_TDM || GameState->isFriendlyFireEnabled) return;

	TArray<AActor*> ActorsToIgnore;
	TArray<AHiRS_Character*> CharacterActors;
	AHiRS_Character::GetAllCharacters(this, CharacterActors);
	const int16 NumOfCharacters = CharacterActors.Num();

	for (int i = 0; i < NumOfCharacters; ++i)
	{
		AHiRS_Character* CheckedCharacter = CharacterActors[i];
		if (!CheckedCharacter->isDead && HiRSOwner->isInTheSameTeam(CheckedCharacter))
		{
			CQParams.AddIgnoredActor(CheckedCharacter);
		}
	}
}


EAuthority AHiRS_Weapon::GetAuthority()
{
	if (AController* Controller = GetInstigatorController())
	{
		if (Role < ROLE_Authority)
		{
			return EAuthority::localClient;
		}
		else
		{
			if (GetWorld()->GetNetMode() == NM_DedicatedServer)
			{
				return EAuthority::dedicatedServer;
			}
			else
			{
				if (Controller->IsLocalController())
				{
					return EAuthority::localServer;
				}
				else
				{
					return EAuthority::server;
				}
			}
		}
	}

	return EAuthority::otherClient;
}

AHiRS_PlayerController * AHiRS_Weapon::GetController()
{
	AController* InstController = GetInstigatorController();

	return InstController ? Cast<AHiRS_PlayerController>(InstController) : NULL;
}

AHiRS_Character * AHiRS_Weapon::GetHirsOwner()
{
	AActor* Owner = GetOwner();
	return Owner ? Cast<AHiRS_Character>(Owner) : nullptr;
}





void AHiRS_Weapon::ChangeCurrentWeapon(EUsedWeapon Weapon)
{
	if (CurrentUsedWeapon == Weapon) return;

	CurrentUsedWeapon = Weapon;

	if (Role < ROLE_Authority)
	{
		ServerChangeCurrentWeapon(Weapon);
	}
}
void AHiRS_Weapon::ServerChangeCurrentWeapon_Implementation(EUsedWeapon Weapon)
{
	ChangeCurrentWeapon(Weapon);
}
bool AHiRS_Weapon::ServerChangeCurrentWeapon_Validate(EUsedWeapon Weapon)
{
	return true;
}

void AHiRS_Weapon::SetWantToShootWeapon(bool bNewWantToShoot)
{
	if (bNewWantToShoot == bWantToShoot) return;

	bWantToShoot = bNewWantToShoot;

	if (Role < ROLE_Authority)
	{
		ServerSetWantToShootWeapon(bNewWantToShoot);
	}

	if (bWantToShoot)
	{
		SetupShootingFunctionsForWeapon();
	}
}
void AHiRS_Weapon::ServerSetWantToShootWeapon_Implementation(bool newWantToShoot)
{
	SetWantToShootWeapon(newWantToShoot);
}
bool AHiRS_Weapon::ServerSetWantToShootWeapon_Validate(bool newWantToShoot)
{
	return true;
}

void AHiRS_Weapon::UseSpecialWeapon()
{

	if (Role < ROLE_Authority)
	{
		ServerUseSpecialWeapon();
	}
	
	SetupShootingFunctionsForSpecial();
}
void AHiRS_Weapon::ServerUseSpecialWeapon_Implementation()
{
	UseSpecialWeapon();
}
bool AHiRS_Weapon::ServerUseSpecialWeapon_Validate()
{
	return true;
}

float AHiRS_Weapon::RewindTime(FVector &CharacterShift)
{
	if (Role < ROLE_Authority || CurrentAuthority == EAuthority::localServer) return 0.0f;
	AHiRS_Character* OwnerRef = GetHirsOwner();
	if (!OwnerRef) return 0.0f;

	TArray<AHiRS_Character*> Characters;
	AHiRS_Character::GetAllCharacters(this, Characters);

	float ping = OwnerRef->GetPing();

	float RewindPing = ping;
	if (ping > 260) RewindPing += ping / 2;
	else RewindPing += (ping / 9);
	RewindPing /= RewindDevide;

	float RewindTime = GetWorld()->TimeSeconds - RewindPing;
	
	uint8 NumOfPlayers = Characters.Num();
	for (int i = 0; i < NumOfPlayers; ++i)
	{
		if (AHiRS_Character* Character = Characters[i])
		{
			if (Character == OwnerRef)
			{
				FLagPose SPrev, SNext;
				float alpha = Character->GetPoseAtTime(RewindTime, SPrev, SNext);
				CharacterShift = Character->GetMeshLocation() - UKismetMathLibrary::VLerp(SPrev.Location, SNext.Location, alpha);
			}
			else if(!Character->isDead && Character->CanBeDamagedBy(OwnerRef))
			{
				Character->RewindMesh(RewindTime);
			}
		}
	}

	return RewindPing;
}




bool AHiRS_Weapon::SphereTrace(FHitResult & OutHit, FCollisionQueryParams & CQParams, ECollisionChannel CollChanel, FVector TraceStart, FVector TraceEnd, float Radius)
{
	return GetWorld()->SweepSingleByChannel(OutHit, TraceStart, TraceEnd, FQuat(), CollChanel, FCollisionShape::MakeSphere(Radius), CQParams);
}
bool AHiRS_Weapon::WorldTrace(FHitResult & OutHit, FVector TraceStart, FVector TraceEnd)
{
	FCollisionQueryParams TraceParams;
		TraceParams.bTraceAsyncScene = true;
		TraceParams.bTraceComplex = true;
		TraceParams.bReturnPhysicalMaterial = true;

	return 	GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_World, TraceParams);
}

void AHiRS_Weapon::MomentalWeaponShot(UPARAM(ref)FMomentalWeaponData& WeaponData)
{
	if (!bWantToShoot) return;

	AHiRS_Character* OwnerRef = GetHirsOwner();
	
	ECollisionChannel Chanel = ECC_Shot;
	float WorldTime = GWorld->GetTimeSeconds();
	float WeaponShotDelay = CalculateWeaponShotDelay(WeaponData.ShootingSpeed, WeaponData.MinShootingSpeed, WeaponData.StartChangeSpeed, WeaponData.ShotsTillMaxDelay);
	float TimeWhenCanShoot = WeaponLastShotTime + WeaponShotDelay;



	if (TimeWhenCanShoot > WorldTime)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_WeaponFire, this, &AHiRS_Weapon::SetupShootingFunctionsForWeapon, TimeWhenCanShoot - WorldTime, false);
		return;
	}


	FHitResult Hit;
	FName TraceFromSocketName = FName("Muzzle");
	FVector StartSocketLocation = WeaponMesh->GetSocketLocation(TraceFromSocketName);
	FVector TraceStart = WeaponMesh->GetSocketLocation(TraceFromSocketName);

	FCollisionQueryParams TraceParams;
		TraceParams.bTraceAsyncScene = true;
		TraceParams.bTraceComplex = false;
		TraceParams.bReturnPhysicalMaterial = true;
		TraceParams.AddIgnoredActor(this);

	if (OwnerRef)
	{
		bool WasTimeRewinded = false;

		FVector CharacterShift = FVector::ZeroVector;
		if (RewindTime(CharacterShift) > 0.0f)
		{
			WasTimeRewinded = true;
			Chanel = ECC_RewindShot;
			TraceStart -= CharacterShift;
		}

		TraceParams.AddIgnoredActor(OwnerRef);
		addIgnoredActors(TraceParams);

		OwnerRef->TraceFromCamera(Hit, WeaponData.MaxShotDistance, WasTimeRewinded, CharacterShift);
		FVector TraceEnd = Hit.TraceEnd;;
		if (Hit.bBlockingHit)
		{
			FVector ShotDirection = (Hit.ImpactPoint - TraceStart).GetSafeNormal();
			TraceEnd = Hit.ImpactPoint + ShotDirection * 20;
		}

		GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, Chanel, TraceParams);

		/*TraceEnd = WorldTrace(Hit, TraceStart, TraceEnd) ? Hit.ImpactPoint : Hit.TraceEnd;
		
		FHitResult ActorHitResult;
		if (SphereTrace(ActorHitResult, TraceParams, Chanel, TraceStart, TraceEnd, SphereTraceRadius))
		{
			Hit = ActorHitResult;
		}*/
	}
	else
	{
		FVector TraceEnd = WeaponMesh->GetSocketRotation(TraceFromSocketName).Vector() * WeaponData.MaxShotDistance;
		GWorld->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, Chanel, TraceParams);
	}

	WeaponLastShotTime = WorldTime;
	AddShotsCount(WeaponData.ShotsTillMaxDelay);

	if (CurrentAuthority == EAuthority::localClient)
	{
		EventOnWeaponMomentalShot(Hit);
	}
	else
	{
		MC_OnWeaponMomentalShot(Hit);
	}

	if (Role == ROLE_Authority)
	{
		ForceNetUpdate();
		MomentalDamage(WeaponData, Hit);
	}

	if (WeaponData.bIsAutomatic)
	{
		WeaponShotDelay = CalculateWeaponShotDelay(WeaponData.ShootingSpeed, WeaponData.MinShootingSpeed, WeaponData.StartChangeSpeed, WeaponData.ShotsTillMaxDelay);
		GetWorldTimerManager().SetTimer(TimerHandle_WeaponFire, this, &AHiRS_Weapon::SetupShootingFunctionsForWeapon, WeaponShotDelay, false);
	}

}
void AHiRS_Weapon::MC_OnWeaponMomentalShot_Implementation(FHitResult HitResult)
{
	if (CurrentAuthority != EAuthority::dedicatedServer && CurrentAuthority != EAuthority::localClient)
	{
		EventOnWeaponMomentalShot(HitResult);
	}	
}

AHiRS_Projectile * AHiRS_Weapon::FireProjectile(const FProjectileWeaponData & WeaponData)
{
	FName SpawnPointName = FName(TEXT("Muzzle"));

	FVector MuzzleLocation = WeaponMesh->GetSocketLocation(SpawnPointName);
	FVector SpawnPosition = MuzzleLocation;
	FRotator SpawnRotation;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AHiRS_Character* OwnerRef = GetHirsOwner();

	FHitResult Hit;
	bool DoesRewindBlock = false;

	if (OwnerRef)
	{
		SpawnParams.Instigator = OwnerRef;
		SpawnParams.Owner = OwnerRef;

		if (CurrentAuthority > EAuthority::localServer)
		{
			FVector CharacterShift = FVector::ZeroVector;
			float ping = RewindTime(CharacterShift);

			auto ProjectileDef = WeaponData.ProjectileClass.GetDefaultObject();
			float ProjectileSpeed = ProjectileDef->GetInitialSpeed();
			float ProjectileRadius = ProjectileDef->GetRadius();

			OwnerRef->TraceFromCamera(Hit, 10000.0f, true, CharacterShift);

			SpawnPosition += CharacterShift;
			SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnPosition, Hit.bBlockingHit ? Hit.ImpactPoint : Hit.TraceEnd);
			MuzzleLocation = SpawnPosition;
			//ping = (OwnerRef->GetPing() / 1000.0);
			SpawnPosition += SpawnRotation.Vector() * ping * ProjectileSpeed;

			FCollisionQueryParams QParam;
				QParam.AddIgnoredActor(this);
				QParam.AddIgnoredActor(OwnerRef);
				addIgnoredActors(QParam);
			DoesRewindBlock = SphereTrace(Hit, QParam, ECC_RewindShot, MuzzleLocation, SpawnPosition, ProjectileRadius);
		}
		else
		{
			OwnerRef->TraceFromCamera(Hit, 10000.0f);
			SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnPosition, Hit.bBlockingHit ? Hit.ImpactPoint : Hit.TraceEnd);
		}

	}
	else
	{
		SpawnParams.Owner = this;
		SpawnRotation = WeaponMesh->GetSocketRotation(SpawnPointName);
	}


	AHiRS_Projectile* SpawnedProjectile;

	if (DoesRewindBlock)
	{
		SpawnedProjectile = GetWorld()->SpawnActor<AHiRS_Projectile>(WeaponData.ProjectileClass, Hit.ImpactPoint, SpawnRotation, SpawnParams);
		if (SpawnedProjectile) SpawnedProjectile->EventOnHit(Hit);
	}
	else
	{
		SpawnedProjectile = GetWorld()->SpawnActor<AHiRS_Projectile>(WeaponData.ProjectileClass, SpawnPosition, SpawnRotation, SpawnParams);
	}

	if (SpawnedProjectile)
	{
		CurrentAuthority = GetAuthority();

		if (CurrentAuthority == EAuthority::localClient)
		{
			EventOnWeaponProjectileShot(SpawnedProjectile);
		}
		else
		{
			MC_OnWeaponProjectileShot(SpawnedProjectile);
			ForceNetUpdate();
		}
	}

	return SpawnedProjectile;
}
void AHiRS_Weapon::ProjectileWeaponShot(UPARAM(ref)FProjectileWeaponData & WeaponData)
{
	if (!bWantToShoot || !WeaponData.ProjectileClass) return;

	float WorldTime = GetWorld()->GetTimeSeconds();
	float WeaponShotDelay = CalculateWeaponShotDelay(WeaponData.ShootingSpeed, WeaponData.MinShootingSpeed, WeaponData.StartChangeSpeed, WeaponData.ShotsTillMaxDelay);
	float TimeWhenCanShoot = WeaponLastShotTime + WeaponShotDelay;

	if (TimeWhenCanShoot > WorldTime)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_WeaponFire, this, &AHiRS_Weapon::SetupShootingFunctionsForWeapon, TimeWhenCanShoot - WorldTime, false);
		return;
	}
	
	AHiRS_Character* OwnerRef = GetHirsOwner();
	AHiRS_Projectile* SpawnedProjectile = FireProjectile(WeaponData);

	WeaponLastShotTime = WorldTime;
	AddShotsCount(WeaponData.ShotsTillMaxDelay);

	if (WeaponData.bIsAutomatic)
	{
		WeaponShotDelay = CalculateWeaponShotDelay(WeaponData.ShootingSpeed, WeaponData.MinShootingSpeed, WeaponData.StartChangeSpeed, WeaponData.ShotsTillMaxDelay);
		GetWorldTimerManager().SetTimer(TimerHandle_WeaponFire, this, &AHiRS_Weapon::SetupShootingFunctionsForWeapon, WeaponShotDelay, false);
	}
	
}
void AHiRS_Weapon::SpecialWeaponShot(UPARAM(ref)FProjectileWeaponData & WeaponData)
{
	if (!WeaponData.ProjectileClass) return;

	float WorldTime = GetWorld()->GetTimeSeconds();
	float WeaponShotDelay = WeaponData.ShootingSpeed;
	float TimeWhenCanShoot = SpecialWeaponLastShotTime + WeaponShotDelay;

	if (TimeWhenCanShoot <= WorldTime || SpecialWeaponLastShotTime == 0.0f)
	{
		AHiRS_Character* OwnerRef = GetHirsOwner();
		AHiRS_Projectile* SpawnedProjectile = FireProjectile(WeaponData);

		SpecialWeaponLastShotTime = WorldTime;
	}
}
void AHiRS_Weapon::MC_OnWeaponProjectileShot_Implementation(const class AHiRS_Projectile * Projectile)
{
	if (Projectile)
	{
		if (CurrentAuthority != EAuthority::dedicatedServer && CurrentAuthority != EAuthority::localClient)
		{
			EventOnWeaponProjectileShot(Projectile);
		}
	}
}

void AHiRS_Weapon::MomentalDamage(FMomentalWeaponData & WeaponData, FHitResult & Hit)
{
	float damage = WeaponData.Damage;
	if (damage == 0.0f) return;
	if (Hit.Actor == NULL) return;

	

	if (AHiRS_Character* Character = Cast<AHiRS_Character>(Hit.Actor.Get()))
	{
		if (Hit.BoneName == FName("head"))
		{
			damage *= Character->HeadshotDamageMultiplier;
		}
	}


	TSubclassOf<UDamageType> const ValidDamageTypeClass = UDamageType::StaticClass();
	FPointDamageEvent PointDamageEvent(damage, Hit, Hit.TraceStart, ValidDamageTypeClass);
	
	Hit.Actor->TakeDamage(damage, PointDamageEvent, GetInstigatorController(), this);
}

float AHiRS_Weapon::CalculateWeaponShotDelay(float minDelay, float maxDelay, int StartChangeSpeed, int ShotsTillMaxDelay)
{
	if (ShotsInTheRow < StartChangeSpeed) return minDelay;
	else
	{
		float Alpha = (ShotsInTheRow - StartChangeSpeed) / (float)ShotsTillMaxDelay;
		if (Alpha > 1.0) Alpha = 1.0;
		return FMath::Lerp(minDelay, maxDelay, Alpha);
	}
}
void AHiRS_Weapon::AddShotsCount(int MaxShotsInTheRow)
{
	if (ShotsInTheRow < MaxShotsInTheRow) ShotsInTheRow++;

	GetWorldTimerManager().SetTimer(TimerHandle_ShotsInTheRow, this, &AHiRS_Weapon::DecreaseShotsInTheRowCount, 0.6, false);
}
void AHiRS_Weapon::DecreaseShotsInTheRowCount()
{
	ShotsInTheRow--;

	if (ShotsInTheRow)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_ShotsInTheRow, this, &AHiRS_Weapon::DecreaseShotsInTheRowCount, 0.07, false);
	}
}

/******************** LIFETIME REPLICATED PROPS */
void AHiRS_Weapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(AHiRS_Weapon, WeaponLastShotTime, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHiRS_Weapon, bWantToShoot, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHiRS_Weapon, SpecialWeaponLastShotTime, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHiRS_Weapon, ShotsInTheRow, COND_OwnerOnly);
	DOREPLIFETIME(AHiRS_Weapon, CurrentUsedWeapon);
}
