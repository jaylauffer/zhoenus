// Copyright 1998-2017 Epic Games, Inc. All Rights Reserve

#include "ZapEmProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "ZhoenusPawn.h"
#include "DonutFlyerPawn.h"
#include "ZhoenusPlayerState.h"
#include "DonutFlyerAIController.h"

AZapEmProjectile::AZapEmProjectile() 
{
	// Static reference to the mesh to use for the projectile
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ProjectileMeshAsset(TEXT("/Game/Flying/Meshes/TwinStickProjectile.TwinStickProjectile"));
	// Create mesh component for the projectile sphere
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh0"));
	ProjectileMesh->SetStaticMesh(ProjectileMeshAsset.Object);
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->BodyInstance.SetCollisionProfileName("Projectile");
	ProjectileMesh->OnComponentHit.AddDynamic(this, &AZapEmProjectile::OnHit);		// set up a notification for when this component hits something
	RootComponent = ProjectileMesh;
	ProjectileMesh->SetCastShadow(false);

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement0"));
	ProjectileMovement->UpdatedComponent = ProjectileMesh;
	ProjectileMovement->InitialSpeed = 5000.f;
	ProjectileMovement->MaxSpeed = 5000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f; // No gravity

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void AZapEmProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddForceAtLocation(GetVelocity() * 20.0f, GetActorLocation());

		// If OtherActor(donut) is shooted
		if (ADonutFlyerPawn* pawn = Cast<ADonutFlyerPawn>(OtherActor))
		{
			if (Attacker != nullptr)
			{
				AZhoenusPlayerState* PlayerState = Attacker->GetPlayerState<AZhoenusPlayerState>();
				if (PlayerState != nullptr)
				{
					PlayerState->OnDonutShootedFromMe();
				}
			}
			if (ADonutFlyerAIController* Controller = Cast<ADonutFlyerAIController>(pawn->Controller))
			{
				Controller->currentState = ADonutFlyerAIController::DonutState::SEARCHING;
			}
		}
	}

	Destroy();
}