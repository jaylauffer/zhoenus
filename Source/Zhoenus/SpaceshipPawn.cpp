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
PlaneMesh->SetSimulatePhysics(true);
PlaneMesh->SetEnableGravity(false);

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

UStaticMeshComponent* Mesh = GetPlaneMesh();
const float Mass = Mesh->GetMass();

// apply thrust as a force rather than teleporting the actor
double& Thrust{ CachedInput.W };
CurrentAcceleration = Thrust * Acceleration;
if (!FMath::IsNearlyZero(Thrust))
{
FVector Force = Mesh->GetForwardVector() * (CurrentAcceleration * Mass * -1.f);
Mesh->AddForce(Force);
}

// clamp velocity and track current forward speed
FVector Velocity = Mesh->GetPhysicsLinearVelocity();
FVector LocalVel = Mesh->GetComponentTransform().InverseTransformVectorNoScale(Velocity);
LocalVel.X = FMath::Clamp(LocalVel.X, MinSpeed, MaxSpeed);
CurrentForwardSpeed = LocalVel.X;
Velocity = Mesh->GetComponentTransform().TransformVectorNoScale(LocalVel);
Mesh->SetPhysicsLinearVelocity(Velocity);

// apply torques for 6DOF rotation
double& Pitch{ CachedInput.X };
double& Yaw{ CachedInput.Y };
double& Roll{ CachedInput.Z };
FVector Torque = FVector::ZeroVector;
//Torque += Mesh->GetRightVector() * (Pitch * PitchSpeed * Mass);
//Torque += Mesh->GetUpVector() * (Yaw * TurnSpeed * Mass);
//Torque += Mesh->GetForwardVector() * (Roll * RollSpeed * Mass);
    const FVector inertia = Mesh->GetInertiaTensor(NAME_None);

    Torque += Mesh->GetRightVector() * Pitch * inertia.X * PitchSpeed;
    Torque += Mesh->GetUpVector()    * Yaw   * inertia.Y * TurnSpeed;
    Torque += Mesh->GetForwardVector()* Roll  * inertia.Z * RollSpeed;

if (FMath::Abs(Pitch) < 0.2f)
{
Torque += Mesh->GetRightVector() * (-GetActorRotation().Pitch * AutoCorrectRate * Mass);
}
if (FMath::Abs(Roll) < 0.2f)
{
Torque += Mesh->GetForwardVector() * (-GetActorRotation().Roll * AutoCorrectRate * Mass);
}
Mesh->AddTorqueInDegrees(Torque, NAME_None, false);

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




