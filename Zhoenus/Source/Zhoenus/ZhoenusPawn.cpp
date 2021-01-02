// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ZhoenusPawn.h"
#include "ZapEmProjectile.h"
#include "DonutFlyerPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Components/SpotLightComponent.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundBase.h"
#include "DrawDebugHelpers.h"
#include "UnrealNetwork.h"
#include "TimerManager.h"

AZhoenusPawn::AZhoenusPawn()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		ConstructorHelpers::FObjectFinder<UParticleSystem> ThrusterParticles;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Flying/Meshes/UFO.UFO"))
			, ThrusterParticles(TEXT("/Game/Flying/Meshes/DonutFlyer/PS_ConeThruster.PS_ConeThruster"))
			//, ThrusterParticles(TEXT("/Game/FXVarietyPack/Particles/P_ky_magicCircle1.P_ky_magicCircle1"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	//bReplicates = true;
	//SetReplicates(true);
	//SetReplicateMovement(false);
	const FName Name_FlyingMovementComponent(TEXT("FlyingMovementComponent"));
	FlyingMovementComponent = CreateDefaultSubobject<UFlyingMovementComponent>(Name_FlyingMovementComponent);
	// Create static mesh component
	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh0"));
	PlaneMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());	// Set static mesh
	PlaneMesh->BodyInstance.bSimulatePhysics = true;
	PlaneMesh->BodyInstance.bEnableGravity = false;
	RootComponent = PlaneMesh;

	// Cache our sound effect
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/Flying/Audio/TwinStickFire.TwinStickFire"));
	FireSound = FireAudio.Object;

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(RootComponent);	// Attach SpringArm to RootComponent
	SpringArm->TargetArmLength = 350.f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = FVector(-466.f,0.f,60.f);
	SpringArm->bEnableCameraLag = true;	// Do not allow camera to lag
	SpringArm->CameraLagSpeed = 20.f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 143.f;
	SpringArm->ProbeChannel = ECC_Visibility;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);	// Attach the camera
	Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller

	HeadLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Headlight0"));
	HeadLight->SetupAttachment(RootComponent);
	HeadLight->SetInnerConeAngle(0.f);
	HeadLight->SetOuterConeAngle(65.f);
	HeadLight->SetAttenuationRadius(8200.f);
	HeadLight->SetIntensity(40000.f);
	HeadLight->SetRelativeLocation(FVector(40.f, 0.f, 14.f));
	HeadLight->SetIndirectLightingIntensity(10000.f);

	LeftThruster = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("LeftThruster"));
	LeftThruster->SetTemplate(ConstructorStatics.ThrusterParticles.Object);
	LeftThruster->SetupAttachment(RootComponent);
	LeftThruster->SetRelativeLocation(FVector(-69.7f, -31.2f, 12.f));
	LeftThruster->SetRelativeScale3D(FVector(.068f, .068f, .068f));
	//LeftThruster->SetLightColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("a5a5ffa5"))));

	RightThruster = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("RightThruster"));
	RightThruster->SetTemplate(ConstructorStatics.ThrusterParticles.Object);
	RightThruster->SetupAttachment(RootComponent);
	RightThruster->SetRelativeLocation(FVector(-69.7f, 31.2f, 12.f));
	RightThruster->SetRelativeScale3D(FVector(.068f, .068f, .068f));
	////RightThruster->SetLightColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("a5a5ffa5"))));

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

	CurrentTimestamp = 0.f;
	LastClientTimestamp = 0.f;
}

void AZhoenusPawn::BeginPlay()
{
	Super::BeginPlay();
	CachedInput = FQuat::Identity;
	//UE_LOG(LogTemp, Log, TEXT("Begin play %s - location: %s rotation: %s quat: %s"), IsNetMode(NM_Client) ? TEXT("client") : TEXT("server"), *GetActorLocation().ToString(), *GetActorRotation().ToString(), *GetActorQuat().ToString());
	if (UWorld* World = GetWorld())
	{
		if (ensure(FlyingMovementComponent))
		{
			FlyingMovementComponent->ProduceInputDelegate.BindUObject(this, &AZhoenusPawn::ProduceInput);
		}
	}
}

void AZhoenusPawn::Tick(float DeltaSeconds)
{
	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);
	//if (IsNetMode(NM_Client))
	//{
	//	CurrentTimestamp += DeltaSeconds;
	//	float NewDeltaSeconds = DeltaSeconds;
	//	if (SavedMoves.Num() > 0)
	//	{
	//		NewDeltaSeconds = CurrentTimestamp - SavedMoves.Last().Timestamp;
	//	}
	//	int32 i = SavedMoves.Emplace(FMovementData{ GetActorLocation(), FVector::ZeroVector, GetActorQuat(), FQuat::Identity, CurrentAcceleration, TargetPitchSpeed, TargetYawSpeed, TargetRollSpeed, CurrentTimestamp });
	//	FMovementData& NewMove{ SavedMoves[i] };
	//	ApplyMovement(NewDeltaSeconds, NewMove.OldPosition, NewMove.NewPosition, NewMove.OldRotation, NewMove.NewRotation);
	//	if (NewMove.NewPosition != NewMove.OldPosition || NewMove.NewRotation != NewMove.OldRotation)
	//	{
	//		ServerMove(NewMove.OldPosition, NewMove.NewPosition, NewMove.OldRotation, NewMove.NewRotation, NewMove.Acceleration, NewMove.PitchSpeed, NewMove.YawSpeed, NewMove.RollSpeed, CurrentTimestamp);
	//	}
	//	else
	//	{
	//		SavedMoves.Pop();
	//	}
	//	while (SavedMoves.Num() > 20)
	//	{
	//		SavedMoves.Pop();
	//	}
	//}
	//else
	//{
	//}
	FireShot();


}

void AZhoenusPawn::ApplyMovement(float DeltaSeconds, FVector& OldPosition, FVector& NewPosition, FQuat& OldRotation, FQuat& NewRotation)
{
	OldPosition = GetActorLocation();
	OldRotation = GetActorQuat();
	float NewForwardSpeed = CurrentForwardSpeed + (DeltaSeconds * CurrentAcceleration);
	// Clamp between MinSpeed and MaxSpeed
	CurrentForwardSpeed = FMath::Clamp(NewForwardSpeed, MinSpeed, MaxSpeed);
	CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, TargetRollSpeed, DeltaSeconds, 2.f);
	CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, DeltaSeconds, 2.f);
	CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, TargetYawSpeed, DeltaSeconds, 2.f);

	const FVector LocalMove = FVector(CurrentForwardSpeed * DeltaSeconds, 0.f, 0.f);

	// Move plan forwards (with sweep so we stop when we collide with things)
	AddActorLocalOffset(LocalMove, true);

	// Calculate change in rotation this frame
	FRotator DeltaRotation(0, 0, 0);
	DeltaRotation.Pitch = CurrentPitchSpeed * DeltaSeconds;
	DeltaRotation.Yaw = CurrentYawSpeed * DeltaSeconds;
	DeltaRotation.Roll = CurrentRollSpeed * DeltaSeconds;

	// Rotate plane
	AddActorLocalRotation(DeltaRotation.Quaternion());

	NewPosition = GetActorLocation();
	NewRotation = GetActorQuat();
}

void AZhoenusPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	//if (OtherComp->GetOwner()->IsA<ADonutFlyerPawn>())
	if(OtherComp && OtherComp->IsSimulatingPhysics())
	{
		FVector push{ GetActorRotation().Quaternion().Vector() * CurrentForwardSpeed * PlaneMesh->GetMass() };
		OtherComp->AddForceAtLocation(push, HitLocation);
	}
	else
	{
		// Deflect along the surface when we collide.
		FRotator CurrentRotation = GetActorRotation();
		SetActorRotation(FQuat::Slerp(CurrentRotation.Quaternion(), HitNormal.ToOrientationQuat(), 0.025f));
	}
}


void AZhoenusPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    // Check if PlayerInputComponent is valid (not NULL)
	check(PlayerInputComponent);

	// Bind our control axis' to callback functions
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (PC->PlayerInput)
		{
			InputComponent->BindAxis("Thrust", this, &AZhoenusPawn::ThrustInput);
			InputComponent->BindAxis("MoveUp", this, &AZhoenusPawn::MoveUpInput);
			InputComponent->BindAxis("MoveRight", this, &AZhoenusPawn::RotateRightInput);
			InputComponent->BindAxis("RotateRight", this, &AZhoenusPawn::MoveRightInput);
			//if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			//{
			//	InputComponent->BindAxis("DisengageAutoCorrect", this, &AZhoenusPlayerController::DisengageAutoCorrect);
			//	InputComponent->BindAxis("FireWeapon", this, &AZhoenusPlayerController::FireWeapon);
			//}
		}
	}
}

void AZhoenusPawn::ThrustInput(float Val)
{
	CachedInput.W = FMath::Clamp(Val * -1.f, -1.f, 1.f);
	//DrawDebugString(GetWorld(), FVector(0.f, 0.f, 50.f), TEXT("Debug"));
	// Is there any input?
	//if (!FMath::IsNearlyEqual(Val, 0.f))
	//{
	//	CurrentAcceleration = Val * Acceleration * -1.f;
	//}
	//else if (!FMath::IsNearlyEqual(CurrentForwardSpeed, 0.f))
	//{
	//	// If input is not held down, reduce speed
	//	CurrentAcceleration = CurrentForwardSpeed < 0.0f ? 0.5f * Acceleration : -0.5f * Acceleration;
	//}
	LeftThruster->SetVectorParameter(TEXT("color"), FVector(15.0f, .3f, Val * 10.0f + 0.3f));
	RightThruster->SetVectorParameter(TEXT("color"), FVector(15.0f, .3f, Val * 10.0f + 0.3f));
	LeftThruster->SetFloatParameter(TEXT("brightness"), Val * 168.0f + 32.f);
	RightThruster->SetFloatParameter(TEXT("brightness"), Val * 168.0f + 32.f);
}

void AZhoenusPawn::MoveUpInput(float Val)
{
	CachedInput.Z = FMath::Clamp(Val * -1.f, -1.f, 1.f);
	// Is there any updown input?
	//const bool bIsInput = FMath::Abs(Val) > 0.2f;

	//// If not turning, roll to reverse current roll value.
	//TargetPitchSpeed = bIsInput ? (Val * TurnSpeed * -1.f) : (GetActorRotation().Pitch * -1.5f * AutoCorrectRate);
	//TargetPitchSpeed = FMath::RoundToFloat(TargetPitchSpeed * 10.f) / 10.f;
}

void AZhoenusPawn::MoveRightInput(float Val)
{
	CachedInput.Y = FMath::Clamp(Val, -1.f, 1.f);
	// Is there any left/right input?
	//const bool bIsTurning = FMath::Abs(Val) > 0.2f;

	// If not turning, roll to reverse current roll value.
	//TargetYawSpeed = bIsTurning ? (Val * TurnSpeed) : 0.f;
	//TargetYawSpeed = FMath::RoundToFloat(TargetYawSpeed * 10.f) / 10.f;
}

void AZhoenusPawn::RotateRightInput(float Val)
{
	CachedInput.X = FMath::Clamp(Val, -1.f, 1.f);
	//const bool bIsTurning = FMath::Abs(Val) > 0.2f;
	//TargetRollSpeed = bIsTurning ? (Val * RollSpeed) : (GetActorRotation().Roll * -1.5f * AutoCorrectRate);
	//TargetRollSpeed = FMath::RoundToFloat(TargetRollSpeed * 10.f) / 10.f;
}

void AZhoenusPawn::DisengageAutoCorrect(float Val)
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

void AZhoenusPawn::FireWeapon(float Val)
{
	CurrentRateOfFire = Val;
}

void AZhoenusPawn::FireShot()
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
			if (World != NULL)
			{
				// spawn the projectile
				World->SpawnActor<AZapEmProjectile>(SpawnLocation, FireRotation);


				bCanFire = false;
				World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &AZhoenusPawn::ShotTimerExpired, FMath::GetRangeValue(FVector2D(2.3f, .2f), CurrentRateOfFire));
			}
			// try and play the sound if specified
			//if (FireSound != nullptr)
			//{
			//	UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
			//}

		}
	}
}

void AZhoenusPawn::ShotTimerExpired()
{
	bCanFire = true;
}

void AZhoenusPawn::ProduceInput(const int32 DeltaMS, FFlyingMovementInputCmd& Cmd)
{
	if (Controller == nullptr)
	{
		// We don't have a local controller so we can't run the code below. This is ok. Simulated proxies will just use previous input when extrapolating
		return;
	}

	// Zero out input structs in case each path doesnt set each member. This is really all we are filling out here.
	Cmd.MovementInput = FVector::ZeroVector;
	Cmd.RotationInput = FRotator::ZeroRotator;

	Cmd.RotationInput.Yaw = FMath::Abs(CachedInput.Y) > 0.2f ? (CachedInput.Y * TurnSpeed) : 0.f;
	Cmd.RotationInput.Roll = FMath::Abs(CachedInput.X) > 0.2f ? (CachedInput.X * RollSpeed) : (GetActorRotation().Roll * -1.5f * AutoCorrectRate);
	Cmd.RotationInput.Pitch = FMath::Abs(CachedInput.Z) > 0.2f ? (CachedInput.Z * TurnSpeed) : (GetActorRotation().Pitch * -1.5f * AutoCorrectRate);

	Cmd.MovementInput = FVector(CachedInput.W, 0, 0);
}

void AZhoenusPawn::ServerMove_Implementation(const FVector& OldPosition, const FVector& NewPostition
	, const FQuat& OldRotation, const FQuat& NewRotation
	, float InAcceleration, float InPitchSpeed, float InYawSpeed, float InRollSpeed
	, float Timestamp)
{
	//TODO: if new positions or rotations don't match resolve the issue.. with a correction
	CurrentAcceleration = InAcceleration;
	TargetPitchSpeed = InPitchSpeed;
	TargetYawSpeed = InYawSpeed;
	TargetRollSpeed = InRollSpeed;

	float DeltaSeconds = Timestamp - LastClientTimestamp;
	LastClientTimestamp = Timestamp;
	FVector ServerOldPosition;
	FVector ServerNewPosition;
	FQuat ServerOldRotation;
	FQuat ServerNewRotation;
	SetActorLocationAndRotation(OldPosition, OldRotation);
	ApplyMovement(DeltaSeconds, ServerOldPosition, ServerNewPosition, ServerOldRotation, ServerNewRotation);
	if (!ServerOldPosition.Equals(OldPosition))
	{
		UE_LOG(LogTemp, Warning, TEXT("Positions from server (%s) don't match client (%s)"), *ServerOldPosition.ToString(), *OldPosition.ToString());
	}
}

bool AZhoenusPawn::ServerMove_Validate(const FVector& OldPosition, const FVector& NewPostition
	, const FQuat& OldRotation, const FQuat& NewRotation
	, float InAcceleration, float InPitchSpeed, float InYawSpeed, float InRollSpeed
	, float Timestamp)
{
	return true;
}

//void AZhoenusPawn::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//}
