// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "DonutFlyerPawn.h"
#include "DonutFlyerAIController.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/EngineTypes.h"
#include "Sound/SoundBase.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "ZhoenusPawn.h"
#include "ZhoenusPlayerState.h"

ADonutFlyerPawn::ADonutFlyerPawn()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Flying/Meshes/DonutFlyer/DonutFlyer.DonutFlyer"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	PrimaryActorTick.bCanEverTick = true;
	// Create static mesh component
	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh0"));
	PlaneMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());	// Set static mesh
	PlaneMesh->BodyInstance.bSimulatePhysics = true;
	PlaneMesh->BodyInstance.bEnableGravity = false;
	//PlaneMesh->SetAngularDamping(1.f);
	//PlaneMesh->SetLinearDamping(1.f);
	RootComponent = PlaneMesh;

	////Set AIController
	AIControllerClass = ADonutFlyerAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Cache our sound effect
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/Flying/Audio/TwinStickFire.TwinStickFire"));
	FireSound = FireAudio.Object;

	// Set handling parameters
	Acceleration = 500.f;
	TurnSpeed = 50.f;
	RollSpeed = 100.f;
	MaxSpeed = 4000.f;
	MinSpeed = 0.f;
	CurrentForwardSpeed = 0.f;

	CurrentRateOfFire = 0.f;
	GunOffset = FVector{ 90.f, 0.f, 10.f };
	bCanFire = true;

	TargetRot = GetActorRotation();
}

void ADonutFlyerPawn::DisengageAutoCorrect(float Val)
{
	AutoCorrectRate = Val;
	if (FMath::IsNearlyEqual(Val, 0.f))
	{
		GetPlaneMesh()->SetAngularDamping(0.f);
		GetPlaneMesh()->SetLinearDamping(0.f);
	}
	else
	{
		GetPlaneMesh()->SetAngularDamping(2.f * Val);
		GetPlaneMesh()->SetLinearDamping(2.f * Val);
	}
}

void ADonutFlyerPawn::Tick(float DeltaSeconds)
{
	//const FVector LocalMove = FVector(CurrentForwardSpeed * DeltaSeconds, 0.f, 0.f);

	//// Move plan forwards (with sweep so we stop when we collide with things)
	//AddActorLocalOffset(LocalMove, true);

	//// Calculate change in rotation this frame
	//FRotator DeltaRotation(0,0,0);
	//DeltaRotation.Pitch = CurrentPitchSpeed * DeltaSeconds;
	//DeltaRotation.Yaw = CurrentYawSpeed * DeltaSeconds;
	//DeltaRotation.Roll = CurrentRollSpeed * DeltaSeconds;

	//// Rotate plane
	//AddActorLocalRotation(DeltaRotation.Quaternion());

	if (CheckStillInWorld())
	{
		FRotator currentRot{ GetActorRotation() };
		ADonutFlyerAIController* ai{ Cast<ADonutFlyerAIController>(Controller) };
		if (ai)
		{
			switch (ai->currentState)
			{
			case ADonutFlyerAIController::CHASING:
			case ADonutFlyerAIController::LOCKED:
				if (currentRot != TargetRot)
				{
					FQuat target_quaternion = TargetRot.Quaternion();
					FQuat current_quaternion = currentRot.Quaternion();
					FQuat quaternion_difference = target_quaternion * current_quaternion.Inverse();
					FVector angle_speed = quaternion_difference.Euler() * 3.f;
					angle_speed.X *= -1.0f;
					angle_speed.Y *= -1.0f;

					PlaneMesh->SetPhysicsAngularVelocityInDegrees(angle_speed);
				}
				break;
			case ADonutFlyerAIController::STUCK:
				PlaneMesh->AddForce(FVector(0.f, 0.f, 100.f));
				break;

			case ADonutFlyerAIController::HOVERING:
			{
				if (!FMath::IsNearlyZero(GetVelocity().Size()))
				{
					PlaneMesh->AddForce(-GetVelocity() * 0.1f);
				}
				//FVector angvel{ GetPlaneMesh()->GetPhysicsAngularVelocity() };
				//if (!FMath::IsNearlyZero(angvel.Size()))
				//{
				//	PlaneMesh->AddTorqueInRadians(-angvel * 0.1f);
				//}
			}
			break;
			}
		}
		GetPlaneMesh()->SetPhysicsLinearVelocity(GetActorForwardVector() * CurrentForwardSpeed);
		//const FVector LocalMove = FVector(CurrentForwardSpeed * DeltaSeconds, 0.f, 0.f);
		// Move forwards (with sweep so we stop when we collide with things)
		//AddActorLocalOffset(LocalMove, true);
		//DrawDebugSphere(GetWorld(), GetActorLocation(), 1350.f, 20, FColor::Red, true);
		FireShot();
	}


	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);
}

void ADonutFlyerPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	//if (OtherComp->GetOwner()->IsRootComponentMovable())
	if (OtherComp && OtherComp->IsSimulatingPhysics())
	{
		FVector push{ GetActorRotation().Quaternion().Vector() * CurrentForwardSpeed * PlaneMesh->GetMass() };
		OtherComp->AddForceAtLocation(push, HitLocation);

		if (AZhoenusPawn* pawn = Cast<AZhoenusPawn>(Other))
		{
			AZhoenusPlayerState* ps = pawn->GetPlayerState<AZhoenusPlayerState>();
			if (ps != nullptr)
			{
				ps->OnDonutHitFromMe(push.Size());
			}
		}
	}
	else
	{
		// Deflect along the surface when we collide.
		FRotator CurrentRotation = GetActorRotation();
		SetActorRotation(FQuat::Slerp(CurrentRotation.Quaternion(), HitNormal.ToOrientationQuat(), 0.025f));
	}
}

void ADonutFlyerPawn::ThrustInput(float Val)
{
	//DrawDebugString(GetWorld(), FVector(0.f, 0.f, 50.f), TEXT("Debug"));
	// Is there any input?
	bool bHasInput = !FMath::IsNearlyEqual(Val, 0.f);
	// If input is not held down, reduce speed
	float CurrentAcc = bHasInput ? (Val * Acceleration * -1.f) : (-0.5f * Acceleration);
	// Calculate new speed
	float NewForwardSpeed = CurrentForwardSpeed + (GetWorld()->GetDeltaSeconds() * CurrentAcc);
	// Clamp between MinSpeed and MaxSpeed
	CurrentForwardSpeed = FMath::Clamp(NewForwardSpeed, MinSpeed, MaxSpeed);
}

void ADonutFlyerPawn::MoveUpInput(float Val)
{

	// When steering, we decrease pitch slightly
	//TargetPitchSpeed += (FMath::Abs(CurrentYawSpeed) * -0.2f);

	// Is there any updown input?
	const bool bIsInput = FMath::Abs(Val) > 0.2f;

	// If not turning, roll to reverse current roll value.
	float TargetPitchSpeed = bIsInput ? (Val * TurnSpeed * -1.f) : (GetActorRotation().Pitch * -1.5f * AutoCorrectRate);


	// Smoothly interpolate to target pitch speed
	CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void ADonutFlyerPawn::MoveRightInput(float Val)
{
	// Is there any left/right input?
	const bool bIsTurning = FMath::Abs(Val) > 0.2f;

	// If not turning, roll to reverse current roll value.
	float TargetYawSpeed = bIsTurning ? (Val * TurnSpeed) : 0.f; 


	// Smoothly interpolate to target yaw speed
	CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, TargetYawSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void ADonutFlyerPawn::RotateRightInput(float Val)
{
	const bool bIsTurning = FMath::Abs(Val) > 0.2f;
	float TargetRollSpeed = bIsTurning ? (Val * RollSpeed) : (GetActorRotation().Roll * -1.5f * AutoCorrectRate);
	CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, TargetRollSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void ADonutFlyerPawn::FireWeapon(float Val)
{
	CurrentRateOfFire = Val;
}

void ADonutFlyerPawn::FireShot()
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

			UWorld* const World = GetWorld();
			if (World != NULL)
			{
				// spawn the projectile
			}

			bCanFire = false;
			World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &ADonutFlyerPawn::ShotTimerExpired, FMath::GetRangeValue(FVector2D(2.5f, .3f), CurrentRateOfFire));

			// try and play the sound if specified
			if (FireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
			}

		}
	}
}

void ADonutFlyerPawn::ShotTimerExpired()
{
	bCanFire = true;
}

//this function is called the particle effect which uses the pawn's static mesh
//when the pawn flys into the goal..
void ADonutFlyerPawn::DelayedDestroy(UNiagaraComponent*)
{
	Destroy();
}

void ADonutFlyerPawn::LockTarget(AActor* Truth, const FVector &Location)
{
	ADonutFlyerAIController* eddie{ Cast<ADonutFlyerAIController>(Controller) };
    if(IsValid(eddie))
    {
        eddie->LockTarget(Truth, Location);
    }
}
