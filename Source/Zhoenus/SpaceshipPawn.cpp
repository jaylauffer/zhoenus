// Copyright 2025 Run Rong Games, All Rights Reserved.

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
#include "TimerManager.h"
#include "Widgets/Input/SVirtualJoystick.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

//todo cleanup dependency on SaveThemAllGameInstance..
#include "SaveThemAllGameInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpaceshipPawn, Log, All);

#define ON_SCREEN_DEBUG 1
#ifdef ON_SCREEN_DEBUG
#include <Runtime/Engine/Classes/Engine/Engine.h>
#define ScreenDebug2(text) if(GEngine)GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White, text)
#else
#define ScreenDebug2(text) 
#endif

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
	PitchSpeed = 50.f;
	MaxSpeed = 3000.f;
	MinSpeed = -1000.f;
	CurrentForwardSpeed = 0.f;

        CurrentRateOfFire = 0.f;
        GunOffset = FVector{ 90.f, 0.f, 10.f };
        bCanFire = true;

}


void ASpaceshipPawn::BeginPlay()
{
	Super::BeginPlay();
	CachedInput = FQuat::Identity;
	CachedInput.W = 0.f;

	// Set handling parameters
	USaveThemAllGameInstance* gi{ Cast<USaveThemAllGameInstance>(GetGameInstance()) };
	if (gi)
	{
		const FShipStats& stats{ gi->shipStats };
		Acceleration = stats.ForwardAcceleration;
		TurnSpeed = stats.YawAcceleration;
		RollSpeed = stats.RollAcceleration;
		PitchSpeed = stats.PitchAcceleration;
		MaxSpeed = stats.MaxSpeed;
		MinSpeed = stats.MinSpeed;
	}
        //UE_LOG(LogSpaceshipPawn, Log, TEXT("Begin play - location: %s rotation: %s quat: %s"), *GetActorLocation().ToString(), *GetActorRotation().ToString(), *GetActorQuat().ToString());

	// Set up the Enhanced Input Subsystem
	if (APlayerController* PController = Cast<APlayerController>(GetController()))
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PController->GetLocalPlayer());

		if (Subsystem)
		{
			// Add your input mapping context here
			Subsystem->AddMappingContext(ShipInputMappingContext, 0);
		}
	}
}

void ASpaceshipPawn::Tick(float DeltaSeconds)
{
        // Call any parent class Tick implementation
        Super::Tick(DeltaSeconds);
       double& Thrust{ CachedInput.W };
       bool bHasInput = !FMath::IsNearlyEqual(Thrust, 0.f);
       CurrentAcceleration = bHasInput ? (Thrust * Acceleration * -1.f) : (CurrentForwardSpeed < 0.0 ? 0.5f * Acceleration : -0.5f * Acceleration);

       // Apply thrust using physics instead of teleporting the actor
       const FVector Force = GetActorForwardVector() * CurrentAcceleration * PlaneMesh->GetMass();
       PlaneMesh->AddForce(Force);

       // Update our stored forward speed from the physics state and clamp to limits
       CurrentForwardSpeed = FVector::DotProduct(PlaneMesh->GetComponentVelocity(), GetActorForwardVector());
       float ClampedSpeed = FMath::Clamp(CurrentForwardSpeed, MinSpeed, MaxSpeed);
       if (!FMath::IsNearlyEqual(CurrentForwardSpeed, ClampedSpeed))
       {
               FVector Vel = PlaneMesh->GetComponentVelocity();
               FVector Forward = GetActorForwardVector();
               Vel += (ClampedSpeed - CurrentForwardSpeed) * Forward;
               PlaneMesh->SetPhysicsLinearVelocity(Vel);
               CurrentForwardSpeed = ClampedSpeed;
       }

        // Calculate change in rotation this frame
        double& Pitch{ CachedInput.X };
        double& Yaw{ CachedInput.Y };
        double& Roll{ CachedInput.Z };

        const bool PitchInput = FMath::Abs(Pitch) > 0.2f;
        TargetPitchSpeed = PitchInput ? (Pitch * PitchSpeed * -1.f) : (GetActorRotation().Pitch * -1.5f * AutoCorrectRate);
        CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, DeltaSeconds, 2.f);

        const bool YawInput = FMath::Abs(Yaw) > 0.2f;
        TargetYawSpeed = YawInput ? (Yaw * TurnSpeed) : 0.f;
        CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, TargetYawSpeed, DeltaSeconds, 2.f);

        const bool RollInput = FMath::Abs(Roll) > 0.2f;
        TargetRollSpeed = RollInput ? (Roll * RollSpeed) : (GetActorRotation().Roll * -1.5f * AutoCorrectRate);
        CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, TargetRollSpeed, DeltaSeconds, 2.f);

       // Apply rotational forces through physics
       FVector Torque = GetActorRightVector() * CurrentPitchSpeed;
       Torque += GetActorUpVector() * CurrentYawSpeed;
       Torque += GetActorForwardVector() * CurrentRollSpeed;
       PlaneMesh->AddTorqueInDegrees(Torque * PlaneMesh->GetMass());

       // Fire weapons if necessary
       FireShot();
}

void ASpaceshipPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	//if (OtherComp->GetOwner()->IsA<ADonutFlyerPawn>())
	if(OtherComp && OtherComp->IsSimulatingPhysics())
	{
		//TODO: add condition for hitting opposite team
		FVector push{ GetActorRotation().Quaternion().Vector() * CurrentForwardSpeed * PlaneMesh->GetMass() };
		OtherComp->AddForceAtLocation(push, HitLocation);
#if WITH_EDITOR
		UE_LOG(LogSpaceshipPawn, Log, TEXT("Collision - other: %s .. me: %s -- push %s - %g %g"), Other?*Other->GetName():TEXT("--unknown--"), *GetName(), *push.ToString(), CurrentForwardSpeed, PlaneMesh->GetMass());
#endif
	}
       else
       {
               // Use an impulse to deflect instead of teleporting rotation
               FVector DeflectImpulse = HitNormal * NormalImpulse.Size();
               PlaneMesh->AddImpulseAtLocation(DeflectImpulse, HitLocation);
               //UE_LOG(LogSpaceshipPawn, Log, TEXT("Slide - other: %s .. me: %s"), Other?*Other->GetName():TEXT("--unknown--"), *GetName());
       }
}


void ASpaceshipPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    // Check if PlayerInputComponent is valid (not NULL)
	check(PlayerInputComponent);

	// Bind our control axis' to callback functions
	//TODO: make this a collaborative experience
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
		if (EnhancedInputComponent)
		{
			EnhancedInputComponent->BindAction(ThrustAction, ETriggerEvent::Triggered, this, &ASpaceshipPawn::ThrustInput);
            EnhancedInputComponent->BindAction(ThrustAction, ETriggerEvent::Completed, this, &ASpaceshipPawn::ThrustInput);
			EnhancedInputComponent->BindAction(PitchAction, ETriggerEvent::Triggered, this, &ASpaceshipPawn::PitchInput);
            EnhancedInputComponent->BindAction(PitchAction, ETriggerEvent::Completed, this, &ASpaceshipPawn::PitchInput);
			EnhancedInputComponent->BindAction(YawAction, ETriggerEvent::Triggered, this, &ASpaceshipPawn::YawInput);
            EnhancedInputComponent->BindAction(YawAction, ETriggerEvent::Completed, this, &ASpaceshipPawn::YawInput);
			EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &ASpaceshipPawn::RollInput);
            EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Completed, this, &ASpaceshipPawn::RollInput);
			EnhancedInputComponent->BindAction(StabilizeAction, ETriggerEvent::Triggered, this, &ASpaceshipPawn::StabilizeInput);
            EnhancedInputComponent->BindAction(StabilizeAction, ETriggerEvent::Completed, this, &ASpaceshipPawn::StabilizeInput);
			// Add more input bindings as needed
		}
		else if (PC->PlayerInput)
		{
			
			InputComponent->BindAxis("Thrust", this, &ASpaceshipPawn::OrigThrustInput);
			InputComponent->BindAxis("MoveUp", this, &ASpaceshipPawn::MoveUpInput);
			InputComponent->BindAxis("MoveRight", this, &ASpaceshipPawn::RotateRightInput); //left stick
			InputComponent->BindAxis("RotateRight", this, &ASpaceshipPawn::MoveRightInput); //right
		}
	}
}

void ASpaceshipPawn::OrigThrustInput(float Val)
{
	//UE_LOG(LogSpaceshipPawn, Log, TEXT("Thrust %g"), Val);
	//ScreenDebug(FString::Printf(TEXT("Thrust %g"), Val));
	CachedInput.W = FMath::Clamp(Val, -1.f, 1.f);
}

void ASpaceshipPawn::ThrustInput(const FInputActionValue& Value)
{
	float Thrust = Value.Get<float>();
	CachedInput.W = FMath::Clamp(Thrust, -1.f, 1.f);
}

void ASpaceshipPawn::PitchInput(const FInputActionValue& Value)
{
	float Pitch = Value.Get<float>();
	CachedInput.X = FMath::Clamp(Pitch, -1.f, 1.f);
}

void ASpaceshipPawn::YawInput(const FInputActionValue& Value)
{
	float Yaw = Value.Get<float>();
	CachedInput.Y = FMath::Clamp(Yaw, -1.f, 1.f);
}

void ASpaceshipPawn::RollInput(const FInputActionValue& Value)
{
	float Roll = Value.Get<float>();
	CachedInput.Z = FMath::Clamp(Roll, -1.f, 1.f);
}

void ASpaceshipPawn::StabilizeInput(const FInputActionValue& Value)
{
	AutoCorrectRate =
	StabilityInput.X = Value.Get<float>();
	if (FMath::IsNearlyEqual(AutoCorrectRate, 0.f))
	{
		GetPlaneMesh()->SetAngularDamping(0.f);
		GetPlaneMesh()->SetLinearDamping(0.f);
	}
	else
	{
		GetPlaneMesh()->SetAngularDamping(20.f * AutoCorrectRate);
		GetPlaneMesh()->SetLinearDamping(20.f * AutoCorrectRate);
	}
}

void ASpaceshipPawn::MoveUpInput(float Val)
{
	//Pitch
	//ScreenDebug(FString::Printf(TEXT("Pitch %g"), Val));
	CachedInput.X = FMath::Clamp(Val, -1.f, 1.f);
	// Is there any updown input?

}

void ASpaceshipPawn::MoveRightInput(float Val)
{
	//Yaw
	//ScreenDebug(FString::Printf(TEXT("Yaw %g"), Val));
	CachedInput.Y = FMath::Clamp(Val, -1.f, 1.f);
	// Is there any left/right input?

}

void ASpaceshipPawn::RotateRightInput(float Val)
{
	//Roll
	//ScreenDebug(FString::Printf(TEXT("Roll %g"), Val));
	CachedInput.Z = FMath::Clamp(Val, -1.f, 1.f);
}

void ASpaceshipPawn::OrigDisengageAutoCorrect(float Val)
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




