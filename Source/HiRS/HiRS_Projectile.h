// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HiRS_Projectile.generated.h"

UCLASS()
class HIRS_API AHiRS_Projectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHiRS_Projectile();

	float GetInitialSpeed();
	float GetRadius();
	bool isEnemyActor(AActor* Actor);


	/** Sphere collision component */
	UPROPERTY(BlueprintReadWrite, VisibleDefaultsOnly, Category = Projectile)
		class USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(BlueprintReadWrite, VisibleDefaultsOnly, Category = Movement)
		class UProjectileMovementComponent* ProjectileMovement;

	/** Projectile Mesh */
	UPROPERTY(BlueprintReadWrite, VisibleDefaultsOnly, Category = Movement)
		class UStaticMeshComponent* ProjectileMesh;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnHit"))
		void EventOnHit(FHitResult Hit);

	UFUNCTION()
		void onBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int OtherBodyIndex, bool FromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	
	
};
