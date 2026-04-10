// Copyright 1998-2017 Epic Games, Inc. All Rights Reserve

#include "ZapEmProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "SaveThemAllGameInstance.h"
#include "UObject/ConstructorHelpers.h"
#include "DonutFlyerAIController.h"
#include "DonutFlyerPawn.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "SpaceshipPawn.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "ZhoenusPlayerState.h"

namespace
{
	constexpr float DefaultProjectileRadius = 12.0f;
}

AZapEmProjectile::AZapEmProjectile()
{
	// Static reference to the assets this projectile uses.
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UStaticMesh> ProjectileMeshAsset;
		ConstructorHelpers::FObjectFinder<USoundBase> AggroFeedbackAudio;
		ConstructorHelpers::FObjectFinder<UNiagaraSystem> AggroFeedbackFlare;

		FConstructorStatics()
			: ProjectileMeshAsset(TEXT("/Game/Flying/Meshes/TwinStickProjectile.TwinStickProjectile"))
			, AggroFeedbackAudio(TEXT("/Game/Flying/Audio/TwinStickFire.TwinStickFire"))
			// ShipFlareEmitter is a NiagaraEmitter asset, not a NiagaraSystem.
			// SpawnSystemAtLocation needs a system, so bind the packaged ShipEngineFlare system here.
			, AggroFeedbackFlare(TEXT("/Game/Effects/ShipEngineFlare.ShipEngineFlare"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Create mesh component for the projectile sphere
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh0"));
	ProjectileMesh->SetStaticMesh(ConstructorStatics.ProjectileMeshAsset.Object);
	ProjectileMesh->BodyInstance.SetCollisionProfileName("Projectile");
	ProjectileMesh->OnComponentHit.AddDynamic(this, &AZapEmProjectile::OnHit);
	ProjectileMesh->SetCastShadow(false);
	RootComponent = ProjectileMesh;

	BaseProjectileRadius = ConstructorStatics.ProjectileMeshAsset.Object ? ConstructorStatics.ProjectileMeshAsset.Object->GetBounds().SphereRadius : DefaultProjectileRadius;
	AggroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AggroSphere0"));
	AggroSphere->SetupAttachment(RootComponent);
	AggroSphere->InitSphereRadius(ResolveAggroRadius());
	AggroSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AggroSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AggroSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AggroSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	AggroSphere->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	AggroSphere->SetGenerateOverlapEvents(true);
	AggroSphere->OnComponentBeginOverlap.AddDynamic(this, &AZapEmProjectile::OnAggroSphereBeginOverlap);

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
	Attacker = nullptr;
	AggroFeedbackSound = ConstructorStatics.AggroFeedbackAudio.Object;
	AggroFeedbackFlare = ConstructorStatics.AggroFeedbackFlare.Object;
}

void AZapEmProjectile::BeginPlay()
{
	Super::BeginPlay();
	BaseProjectileScale = ProjectileMesh != nullptr ? ProjectileMesh->GetRelativeScale3D() : FVector::OneVector;

	if (USaveThemAllGameInstance* GameInstance = Cast<USaveThemAllGameInstance>(GetGameInstance()))
	{
		// Radius comes from the firing player's ship customization path.
		// The projectile's threat weights stay in global Donut aggro tuning.
		AggroRadiusMultiplier = GameInstance->shipStats.ProjectileAggroRadiusMultiplier;
		ProjectileNearMissThreat = GameInstance->donutAggroTuning.ProjectileNearMissThreat;
		ProjectileHitThreat = GameInstance->donutAggroTuning.ProjectileHitThreat;
	}

	if (AggroSphere != nullptr)
	{
		AggroSphere->SetSphereRadius(ResolveAggroRadius());
	}
}

void AZapEmProjectile::SetAggroRadiusOverride(const float InAggroRadius)
{
	AggroRadiusOverride = FMath::Max(0.f, InAggroRadius);

	if (AggroSphere != nullptr)
	{
		AggroSphere->SetSphereRadius(ResolveAggroRadius());
	}
}

float AZapEmProjectile::ResolveAggroRadius() const
{
	if (AggroRadiusOverride > 0.f)
	{
		return AggroRadiusOverride;
	}

	return BaseProjectileRadius * AggroRadiusMultiplier;
}

void AZapEmProjectile::TriggerDonutFlyerAggro(ADonutFlyerPawn* Pawn, EDonutAggroEventType EventType, float AggroAmount, bool bCountPlayerShot, const FVector& ContactLocation)
{
	if (!IsValid(Pawn))
	{
		return;
	}

	if (bCountPlayerShot && Attacker != nullptr)
	{
		if (AZhoenusPlayerState* PlayerState = Attacker->GetPlayerState<AZhoenusPlayerState>())
		{
			PlayerState->RecordDonutShot();
		}
	}

	if (ADonutFlyerAIController* Controller = Pawn->GetController<ADonutFlyerAIController>())
	{
		Controller->ApplyAggroEvent(Attacker, EventType, AggroAmount);
	}

	const TWeakObjectPtr<ADonutFlyerPawn> PawnKey(Pawn);
	if (!AggroedDonuts.Contains(PawnKey))
	{
		AggroedDonuts.Add(PawnKey);
		PlayAggroFeedback(ContactLocation);
	}
}

void AZapEmProjectile::PlayAggroFeedback(const FVector& ContactLocation)
{
	if (AggroFeedbackSound != nullptr)
	{
        //This is an interesting opportunity, actually the intent is to play the sound to ourselves so we know we triggered the flyer
        //as well we would like haptic feedback.. .. in a space game with no atmosphere sound won't travel.. however if we play the sound
        //at a location and another flyer overhears it, it could be used to draw their attention... design choices, there are so
        //many opportunities, with multiple development PODs we could iterate and explore each of these to gather real-world feedback
        
        //for now I will pay the sound as audible feedback TODO: haptic feedback
		UGameplayStatics::PlaySound2D(this, AggroFeedbackSound, AggroFeedbackSoundVolume);
	}

	if (AggroFeedbackFlare != nullptr && GetWorld() != nullptr)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			AggroFeedbackFlare,
			ContactLocation,
			GetActorRotation(),
			AggroFlareScale);
	}

	if (ProjectileMesh != nullptr)
	{
		ProjectileMesh->SetRelativeScale3D(BaseProjectileScale * AggroPulseScale);
		GetWorldTimerManager().ClearTimer(TimerHandle_AggroPulseReset);
		GetWorldTimerManager().SetTimer(
			TimerHandle_AggroPulseReset,
			this,
			&AZapEmProjectile::ResetAggroPulse,
			AggroPulseDuration,
			false);
	}
}

void AZapEmProjectile::ResetAggroPulse()
{
	if (ProjectileMesh != nullptr)
	{
		ProjectileMesh->SetRelativeScale3D(BaseProjectileScale);
	}
}

void AZapEmProjectile::OnAggroSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr || OtherActor == this || OtherActor == Attacker)
	{
		return;
	}

	if (ADonutFlyerPawn* Pawn = Cast<ADonutFlyerPawn>(OtherActor))
	{
		const FVector ContactLocation = bFromSweep && !SweepResult.ImpactPoint.IsNearlyZero()
			? FVector(SweepResult.ImpactPoint)
			: Pawn->GetActorLocation();
		TriggerDonutFlyerAggro(Pawn, EDonutAggroEventType::ProjectileNearMiss, ProjectileNearMissThreat, false, ContactLocation);
	}
}

void AZapEmProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddForceAtLocation(GetVelocity() * 20.0f, GetActorLocation());

		if (ADonutFlyerPawn* Pawn = Cast<ADonutFlyerPawn>(OtherActor))
		{
			const FVector ContactLocation = !Hit.ImpactPoint.IsNearlyZero() ? FVector(Hit.ImpactPoint) : Pawn->GetActorLocation();
			TriggerDonutFlyerAggro(Pawn, EDonutAggroEventType::ProjectileHit, ProjectileHitThreat, true, ContactLocation);
		}
	}

	Destroy();
}
