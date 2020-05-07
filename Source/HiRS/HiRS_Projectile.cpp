// Fill out your copyright notice in the Description page of Project Settings.

#include "HiRS_Projectile.h"
#include "HiRS.h"
#include "HiRS_Character.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AHiRS_Projectile::AHiRS_Projectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collision"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	CollisionComp->SetCollisionObjectType(ECollisionChannel::ECC_Projectile);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_World, ECollisionResponse::ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AHiRS_Projectile::onBeginOverlap);
	CollisionComp->OnComponentHit.AddDynamic(this, &AHiRS_Projectile::OnHit);

	RootComponent = CollisionComp;
	
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	ProjectileMesh->SetupAttachment(CollisionComp);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 18000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	bReplicates = true;
}

float AHiRS_Projectile::GetInitialSpeed()
{
	if (ProjectileMovement) return ProjectileMovement->InitialSpeed;
	else return 0.0f;
}

float AHiRS_Projectile::GetRadius()
{
	return CollisionComp->GetScaledSphereRadius();
}

bool AHiRS_Projectile::isEnemyActor(AActor * Actor)
{
	if ( AHiRS_Character* Owner = Cast<AHiRS_Character>(GetOwner()) )
	{
		if (Actor == Owner) return false;
		else
		{
			return Owner->CanBeDamagedBy(Actor);
		}
	}
	
	return true;
}


// Called when the game starts or when spawned
void AHiRS_Projectile::BeginPlay()
{
	Super::BeginPlay();

}


void AHiRS_Projectile::onBeginOverlap(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComponent, int OtherBodyIndex, bool FromSweep, const FHitResult & SweepResult)
{
	if (OtherActor && OtherComponent)
	{
		if (isEnemyActor(OtherActor))
		{
			EventOnHit(SweepResult);
		}
	}
}

void AHiRS_Projectile::OnHit(UPrimitiveComponent * HitComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	if (OtherActor && OtherComp)
	{
		if (isEnemyActor(OtherActor))
		{
			EventOnHit(Hit);
		}
	}
}
