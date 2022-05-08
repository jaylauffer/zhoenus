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

	if (GetNetMode() != NM_Standalone)
	{
		const FName Name_FlyingMovementComponent(TEXT("ZhoenusMovementComponent"));
		FlyingMovementComponent = CreateDefaultSubobject<UZhoenusMovementComponent>(Name_FlyingMovementComponent);
		FlyingMovementComponent->SetMaxMoveSpeed(MaxSpeed);
	}


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
		if (GetNetMode() != NM_Standalone && ensure(FlyingMovementComponent))
		{
			FlyingMovementComponent->ProduceInputDelegate.BindUObject(this, &ASpaceshipPawn::ProduceInput);
		}
	}
}

void ASpaceshipPawn::Tick(float DeltaSeconds)
{
	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);
	if (GetNetMode() == NM_Standalone)
	{
		double& Thrust{ CachedInput.W };
		bool bHasInput = !FMath::IsNearlyEqual(Thrust, 0.f);
		CurrentAcceleration = bHasInput ? (Thrust * Acceleration * -1.f) : (CurrentForwardSpeed < 0.0 ? 0.5f * Acceleration : -0.5f * Acceleration);
		CurrentForwardSpeed += CurrentAcceleration * DeltaSeconds;
		CurrentForwardSpeed = FMath::Clamp(CurrentForwardSpeed, MinSpeed, MaxSpeed);
		const FVector LocalMove = FVector(CurrentForwardSpeed * DeltaSeconds, 0.f, 0.f);
		
		// Move plane forwards (with sweep so we stop when we collide with things)
		AddActorLocalOffset(LocalMove, true);
		
		// Calculate change in rotation this frame
		double& Pitch{ CachedInput.X };
		double& Yaw{ CachedInput.Y };
		double& Roll{ CachedInput.Z };

		const bool PitchInput = FMath::Abs(Pitch) > 0.2f;
		TargetPitchSpeed = PitchInput ? (Pitch * TurnSpeed * -1.f) : (GetActorRotation().Pitch * -1.5f * AutoCorrectRate);
		CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, DeltaSeconds, 2.f);

		const bool YawInput = FMath::Abs(Yaw) > 0.2f;
		TargetYawSpeed = YawInput ? (Yaw * TurnSpeed) : 0.f;
		CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, TargetYawSpeed, DeltaSeconds, 2.f);
		
		const bool RollInput = FMath::Abs(Roll) > 0.2f;
		TargetRollSpeed = RollInput ? (Roll * RollSpeed) : (GetActorRotation().Roll * -1.5f * AutoCorrectRate);
		CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, TargetRollSpeed, DeltaSeconds, 2.f);

		FRotator DeltaRotation(0, 0, 0);
		DeltaRotation.Pitch = CurrentPitchSpeed * DeltaSeconds;
		DeltaRotation.Yaw = CurrentYawSpeed * DeltaSeconds;
		DeltaRotation.Roll = CurrentRollSpeed * DeltaSeconds;
		AddActorLocalRotation(DeltaRotation.Quaternion());
	}
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
	CachedInput.W = FMath::Clamp(Val * 1.f, -1.f, 1.f);
}

void ASpaceshipPawn::MoveUpInput(float Val)
{
	//Pitch
	CachedInput.X = FMath::Clamp(Val * 1.f, -1.f, 1.f);
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




