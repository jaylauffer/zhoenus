// Copyright 2025 Run Rong Games, All Rights Reserved.

#include "SpaceshipPawn.h"
#include "ZapEmProjectile.h"
#include "DonutFlyerPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Sound/SoundBase.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Widgets/Input/SVirtualJoystick.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"

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
		ConstructorHelpers::FObjectFinder<UInputAction> FireAction;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Flying/Meshes/UFO.UFO"))
			, FireAction(TEXT("/Game/Input/Actions/FireInputAction.FireInputAction"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Create static mesh component
	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh0"));
	PlaneMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());	// Set static mesh
	PlaneMesh->BodyInstance.bSimulatePhysics = false;
	PlaneMesh->BodyInstance.bEnableGravity = false;
	RootComponent = PlaneMesh;

	// Cache our sound effect
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/Flying/Audio/TwinStickFire.TwinStickFire"));
	FireSound = FireAudio.Object;

	// Set handling parameters
	Acceleration = 500.f;
	ReverseAcceleration = 200.f;
	TurnSpeed = 50.f;
	RollSpeed = 100.f;
	PitchSpeed = 50.f;
	MaxSpeed = 3000.f;
	MinSpeed = -1000.f;
	CurrentForwardSpeed = 0.f;

	CurrentRateOfFire = 0.f;
	GunOffset = FVector{ 90.f, 0.f, 10.f };
	bCanFire = true;
	FireAction = ConstructorStatics.FireAction.Object;

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
		ReverseAcceleration = stats.ReverseAcceleration;
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
    
    // Sticky mouse decay (only for look axes)
    if (LastInputSource == EInputSource::Mouse && MouseStickDecay > 0.f)
    {
        CachedInput.X = FMath::FInterpTo(CachedInput.X, 0.f, DeltaSeconds, MouseStickDecay); // Pitch
        CachedInput.Y = FMath::FInterpTo(CachedInput.Y, 0.f, DeltaSeconds, MouseStickDecay); // Yaw
    }
    
        double& Thrust{ CachedInput.W };
        bool bHasInput = !FMath::IsNearlyEqual(Thrust, 0.f);
        const float AppliedAcceleration =
            Thrust < 0.f ? Acceleration : ReverseAcceleration;
        CurrentAcceleration = bHasInput ? (Thrust * AppliedAcceleration * -1.f) : (CurrentForwardSpeed < 0.0 ? 0.5f * ReverseAcceleration : -0.5f * Acceleration);
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
        TargetPitchSpeed = PitchInput ? (Pitch * PitchSpeed * -1.f) : (GetActorRotation().Pitch * -1.5f * AutoCorrectRate);
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
        FireShot();
}

void ASpaceshipPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	//if (OtherComp->GetOwner()->IsA<ADonutFlyerPawn>())
	if(OtherComp && OtherComp->IsSimulatingPhysics())
	{
		float Mass = PlaneMesh->IsSimulatingPhysics() ? PlaneMesh->GetMass() : KinematicMassKg;
		//TODO: add condition for hitting opposite team
		FVector push{ GetActorRotation().Quaternion().Vector() * CurrentForwardSpeed * Mass };
		OtherComp->AddForceAtLocation(push, HitLocation);
#if WITH_EDITOR
		UE_LOG(LogSpaceshipPawn, Log, TEXT("Collision - other: %s .. me: %s -- push %s - %g %g"), Other?*Other->GetName():TEXT("--unknown--"), *GetName(), *push.ToString(), CurrentForwardSpeed, Mass);
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
			EnhancedInputComponent->BindAction(ThrustAction, ETriggerEvent::Canceled, this, &ASpaceshipPawn::ThrustInput);
			EnhancedInputComponent->BindAction(PitchAction, ETriggerEvent::Triggered, this, &ASpaceshipPawn::PitchInput);
            EnhancedInputComponent->BindAction(PitchAction, ETriggerEvent::Completed, this, &ASpaceshipPawn::PitchInput);
			EnhancedInputComponent->BindAction(PitchAction, ETriggerEvent::Canceled, this, &ASpaceshipPawn::PitchInput);
			EnhancedInputComponent->BindAction(YawAction, ETriggerEvent::Triggered, this, &ASpaceshipPawn::YawInput);
            EnhancedInputComponent->BindAction(YawAction, ETriggerEvent::Completed, this, &ASpaceshipPawn::YawInput);
			EnhancedInputComponent->BindAction(YawAction, ETriggerEvent::Canceled, this, &ASpaceshipPawn::YawInput);
			EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &ASpaceshipPawn::RollInput);
            EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Completed, this, &ASpaceshipPawn::RollInput);
			EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Canceled, this, &ASpaceshipPawn::RollInput);
			EnhancedInputComponent->BindAction(StabilizeAction, ETriggerEvent::Triggered, this, &ASpaceshipPawn::StabilizeInput);
            EnhancedInputComponent->BindAction(StabilizeAction, ETriggerEvent::Completed, this, &ASpaceshipPawn::StabilizeInput);
			EnhancedInputComponent->BindAction(StabilizeAction, ETriggerEvent::Canceled, this, &ASpaceshipPawn::StabilizeInput);
			if (FireAction)
			{
				EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ASpaceshipPawn::StartFireInput);
				EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ASpaceshipPawn::StartFireInput);
				EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ASpaceshipPawn::StopFireInput);
				EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Canceled, this, &ASpaceshipPawn::StopFireInput);
			}
			// Add more input bindings as needed
            
            // NEW: accumulate mouse deltas into a persistent "virtual stick"
            if (MouseXAction) { EnhancedInputComponent->BindAction(MouseXAction, ETriggerEvent::Triggered, this, &ASpaceshipPawn::OnMouseX); }
            if (MouseYAction) { EnhancedInputComponent->BindAction(MouseYAction, ETriggerEvent::Triggered, this, &ASpaceshipPawn::OnMouseY); }
		}
//		else if (PC->PlayerInput)
//		{
//			
//			InputComponent->BindAxis("Thrust", this, &ASpaceshipPawn::OrigThrustInput);
//			InputComponent->BindAxis("MoveUp", this, &ASpaceshipPawn::MoveUpInput);
//			InputComponent->BindAxis("MoveRight", this, &ASpaceshipPawn::RotateRightInput); //left stick
//			InputComponent->BindAxis("RotateRight", this, &ASpaceshipPawn::MoveRightInput); //right
//		}
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
	LastInputSource = EInputSource::Gamepad;
	float Thrust = Value.Get<float>();
	CachedInput.W = FMath::Clamp(Thrust, -1.f, 1.f);
}

void ASpaceshipPawn::PitchInput(const FInputActionValue& Value)
{
	LastInputSource = EInputSource::Gamepad;
	float Pitch = Value.Get<float>();
	CachedInput.X = FMath::Clamp(Pitch, -1.f, 1.f);
}

void ASpaceshipPawn::YawInput(const FInputActionValue& Value)
{
	LastInputSource = EInputSource::Gamepad;
	float Yaw = Value.Get<float>();
	CachedInput.Y = FMath::Clamp(Yaw, -1.f, 1.f);
}

void ASpaceshipPawn::RollInput(const FInputActionValue& Value)
{
	LastInputSource = EInputSource::Gamepad;
	float Roll = Value.Get<float>();
	CachedInput.Z = FMath::Clamp(Roll, -1.f, 1.f);
}

void ASpaceshipPawn::StabilizeInput(const FInputActionValue& Value)
{
	LastInputSource = EInputSource::Gamepad;
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

void ASpaceshipPawn::StartFireInput(const FInputActionValue& Value)
{
	CurrentRateOfFire = Value.GetMagnitude();
}

void ASpaceshipPawn::StopFireInput()
{
	CurrentRateOfFire = 0.f;
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

void ASpaceshipPawn::OnMouseX(const FInputActionValue& Value)
{
    LastInputSource = EInputSource::Mouse;
    // Mouse X → yaw stick (CachedInput.Y)
    const float dx = Value.Get<float>();
    CachedInput.Y = FMath::Clamp(CachedInput.Y + dx * MouseStickSensitivity, -1.f, 1.f);
}

void ASpaceshipPawn::OnMouseY(const FInputActionValue& Value)
{
    LastInputSource = EInputSource::Mouse;
    // Mouse Y → pitch stick (CachedInput.X); invert if you prefer flight-style
    const float dy = Value.Get<float>();
    CachedInput.X = FMath::Clamp(CachedInput.X + dy * MouseStickSensitivity, -1.f, 1.f);
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
	CurrentRateOfFire = FMath::Max(0.f, Val);
}

FVector ASpaceshipPawn::GetProjectileSpawnLocation() const
{
	return GetActorLocation() + GetActorForwardVector().Rotation().RotateVector(GunOffset);
}

FVector ASpaceshipPawn::GetProjectileFireDirection() const
{
	return GetActorForwardVector().GetSafeNormal();
}

FVector ASpaceshipPawn::GetProjectileAimPoint(const float TraceDistance) const
{
	const FVector SpawnLocation = GetProjectileSpawnLocation();
	const FVector TraceEnd = SpawnLocation + GetProjectileFireDirection() * FMath::Max(0.f, TraceDistance);

	if (UWorld* World = GetWorld())
	{
		FHitResult Hit;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ProjectileAimPoint), false, this);
		QueryParams.AddIgnoredActor(this);
		for (TActorIterator<ADonutFlyerPawn> DonutIt(World); DonutIt; ++DonutIt)
		{
			if (IsValid(*DonutIt))
			{
				QueryParams.AddIgnoredActor(*DonutIt);
			}
		}
		for (TActorIterator<AZapEmProjectile> ProjectileIt(World); ProjectileIt; ++ProjectileIt)
		{
			if (IsValid(*ProjectileIt))
			{
				QueryParams.AddIgnoredActor(*ProjectileIt);
			}
		}
		if (World->LineTraceSingleByChannel(Hit, SpawnLocation, TraceEnd, ECC_Visibility, QueryParams))
		{
			return Hit.ImpactPoint;
		}
	}

	return TraceEnd;
}

float ASpaceshipPawn::GetProjectileAggroRadius() const
{
	return 0.f;
}

void ASpaceshipPawn::FireShot()
{
	// If we it's ok to fire again
	if (bCanFire == true)
	{
		// If we are pressing fire stick in a direction
		if (CurrentRateOfFire > 0.0f)
		{
			const FVector FireDirection = GetProjectileFireDirection();
			const FRotator FireRotation = FireDirection.Rotation();
			const FVector SpawnLocation = GetProjectileSpawnLocation();

			UWorld* const World{ GetWorld() };
			if (World != nullptr)
			{
				// spawn the projectile
				AZapEmProjectile* zap = World->SpawnActor<AZapEmProjectile>(SpawnLocation, FireRotation);
				if (zap != nullptr)
				{
					zap->Attacker = this;
					zap->SetAggroRadiusOverride(GetProjectileAggroRadius());
				}

				bCanFire = false;
				World->GetTimerManager().SetTimer(
					TimerHandle_ShotTimerExpired,
					this,
					&ASpaceshipPawn::ShotTimerExpired,
					FMath::GetRangeValue(FVector2D(2.3f, .2f), CurrentRateOfFire));
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
