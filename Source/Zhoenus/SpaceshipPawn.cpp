// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SpaceshipPawn.h"
#include "ZapEmProjectile.h"
#include "DonutFlyerPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundBase.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Widgets/Input/SVirtualJoystick.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpaceshipPawn, Log, All);

ASpaceshipPawn::ASpaceshipPawn(const FObjectInitializer &initializer)
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Flying/Meshes/UFO.UFO"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Create static mesh component
	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh0"));
	PlaneMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());	// Set static mesh
	PlaneMesh->BodyInstance.bSimulatePhysics = true;
	PlaneMesh->BodyInstance.bEnableGravity = false;
	RootComponent = PlaneMesh;

	const FName Name_FlyingMovementComponent(TEXT("ZhoenusMovementComponent"));
	FlyingMovementComponent = CreateDefaultSubobject<UZhoenusMovementComponent>(Name_FlyingMovementComponent);

	// Cache our sound effect
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/Flying/Audio/TwinStickFire.TwinStickFire"));
	FireSound = FireAudio.Object;

	// Set handling parameters
	Acceleration = 500.f;
	TurnSpeed = 50.f;
	RollSpeed = 100.f;
	MaxSpeed = 3000.f;
	MinSpeed = -1000.f;
	CurrentForwardSpeed = 0.f;

	FlyingMovementComponent->SetMaxMoveSpeed(MaxSpeed);

	CurrentRateOfFire = 0.f;
	GunOffset = FVector{ 90.f, 0.f, 10.f };
	bCanFire = true;

	bReplicates = true;

}

void ASpaceshipPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, IsDonutTarget);
}


void ASpaceshipPawn::BeginPlay()
{
	Super::BeginPlay();
	CachedInput = FQuat::Identity;
	//UE_LOG(LogSpaceshipPawn, Log, TEXT("Begin play %s - location: %s rotation: %s quat: %s"), IsNetMode(NM_Client) ? TEXT("client") : TEXT("server"), *GetActorLocation().ToString(), *GetActorRotation().ToString(), *GetActorQuat().ToString());
	if (UWorld* World = GetWorld())
	{
		if (ensure(FlyingMovementComponent))
		{
			FlyingMovementComponent->ProduceInputDelegate.BindUObject(this, &ASpaceshipPawn::ProduceInput);
		}
	}
}

void ASpaceshipPawn::Tick(float DeltaSeconds)
{
	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);

	FireShot();
}

void ASpaceshipPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	//if (OtherComp->GetOwner()->IsA<ADonutFlyerPawn>())
	if(OtherComp && OtherComp->IsSimulatingPhysics())
	{
		//TODO: add condition for hitting opposite team
		FVector push{ GetActorRotation().Quaternion().Vector() * GetVelocity().Size() * PlaneMesh->GetMass() };
		OtherComp->AddForceAtLocation(push, HitLocation);
//		UE_LOG(LogSpaceshipPawn, Log, TEXT("Collision - other: %s .. me: %s -- push %s - %g %g"), Other?*Other->GetName():TEXT("--unknown--"), *GetName(), *push.ToString(), GetVelocity().Size(), PlaneMesh->GetMass());
	}
	else
	{
		// Deflect along the surface when we collide.
		FRotator CurrentRotation = GetActorRotation();
		SetActorRotation(FQuat::Slerp(CurrentRotation.Quaternion(), HitNormal.ToOrientationQuat(), 0.025f));
		//UE_LOG(LogSpaceshipPawn, Log, TEXT("Slide - other: %s .. me: %s"), Other?*Other->GetName():TEXT("--unknown--"), *GetName());
	}
}


void ASpaceshipPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    // Check if PlayerInputComponent is valid (not NULL)
	check(PlayerInputComponent);

	// Bind our control axis' to callback functions
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (PC->PlayerInput)
		{
			InputComponent->BindAxis("Thrust", this, &ASpaceshipPawn::ThrustInput);
			InputComponent->BindAxis("MoveUp", this, &ASpaceshipPawn::MoveUpInput);
			InputComponent->BindAxis("MoveRight", this, &ASpaceshipPawn::RotateRightInput);
			InputComponent->BindAxis("RotateRight", this, &ASpaceshipPawn::MoveRightInput);
		}
	}
}

void ASpaceshipPawn::ThrustInput(float Val)
{
	CachedInput.W = FMath::Clamp(Val * -1.f, -1.f, 1.f);
}

void ASpaceshipPawn::MoveUpInput(float Val)
{
	//Pitch
	CachedInput.X = FMath::Clamp(Val * -1.f, -1.f, 1.f);
	// Is there any updown input?

}

void ASpaceshipPawn::MoveRightInput(float Val)
{
	//Yaw
	CachedInput.Y = FMath::Clamp(Val, -1.f, 1.f);
	// Is there any left/right input?

}

void ASpaceshipPawn::RotateRightInput(float Val)
{
	//Roll
	CachedInput.Z = FMath::Clamp(Val, -1.f, 1.f);
}

void ASpaceshipPawn::DisengageAutoCorrect(float Val)
{
	AutoCorrectRate = Val;
	if (FMath::IsNearlyEqual(Val, 0.f))
	{
		GetPlaneMesh()->SetAngularDamping(0.f);
		GetPlaneMesh()->SetLinearDamping(0.f);
	}
	else
	{
		GetPlaneMesh()->SetAngularDamping(20.f * Val);
		GetPlaneMesh()->SetLinearDamping(20.f * Val);
	}
}

void ASpaceshipPawn::FireWeapon(float Val)
{
	CurrentRateOfFire = Val;
}

void ASpaceshipPawn::FireShot()
{
	// If we it's ok to fire again
	if (bCanFire == true)
	{
		// If we are pressing fire stick in a direction
		if (CurrentRateOfFire > 0.0f)
		{
			const FRotator FireRotation = GetActorForwardVector().Rotation();
			// Spawn projectile at an offset from this pawn
			const FVector SpawnLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);

			UWorld* const World{ GetWorld() };
			if (World != nullptr)
			{
				// spawn the projectile
				AZapEmProjectile* zap = World->SpawnActor<AZapEmProjectile>(SpawnLocation, FireRotation);
				zap->Attacker = this;

				bCanFire = false;
				World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &ASpaceshipPawn::ShotTimerExpired, FMath::GetRangeValue(FVector2D(2.3f, .2f), CurrentRateOfFire));
			}
			// try and play the sound if specified
			//if (FireSound != nullptr)
			//{
			//	UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
			//}

		}
	}
}

void ASpaceshipPawn::ShotTimerExpired()
{
	bCanFire = true;
}

void ASpaceshipPawn::ProduceInput(const int32 DeltaMS, FZhoenusMovementInputCmd& Cmd)
{
	Cmd.RotationInput = CachedInput;
	Cmd.MovementInput = FVector(AutoCorrectRate, CurrentRateOfFire, 0);
}




