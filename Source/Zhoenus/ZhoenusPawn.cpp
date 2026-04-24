// Copyright 2025 Run Rong Games, All Rights Reserved.

#include "ZhoenusPawn.h"
#include "ZapEmProjectile.h"
#include "DonutFlyerPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/MaterialBillboardComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Components/SpotLightComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundBase.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Widgets/Input/SVirtualJoystick.h"

DEFINE_LOG_CATEGORY_STATIC(LogZhoenusPawn, Log, All);



AZhoenusPawn::AZhoenusPawn(const FObjectInitializer &initializer) : Super(initializer)
{
	PrimaryActorTick.bCanEverTick = true;

	//NiagaraSystem'/Game/Effects/ShipEngineFlare.ShipEngineFlare'

	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		//ConstructorHelpers::FObjectFinder<UNiagaraSystem> ThrusterParticles;
		ConstructorHelpers::FObjectFinder<USoundBase> FireAudio;
		FConstructorStatics()
			//: ThrusterParticles{ TEXT("/Game/Effects/ShipEngineFlare.ShipEngineFlare") }
			: FireAudio{ TEXT("/Game/Flying/Audio/TwinStickFire.TwinStickFire") }
		{
		}
	};
	static FConstructorStatics Statics;

	// Cache our sound effect
	FireSound = Statics.FireAudio.Object;

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(RootComponent);	// Attach SpringArm to RootComponent
	SpringArm->TargetArmLength = 350.f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = FVector(-466.f,0.f,60.f);
	SpringArm->bEnableCameraLag = true;	// Do not allow camera to lag
	SpringArm->CameraLagSpeed = 20.f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 7.5f;
	SpringArm->ProbeChannel = ECC_Camera;

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

	AimProjectorComponent = CreateDefaultSubobject<UMaterialBillboardComponent>(TEXT("AimProjector0"));
	AimProjectorComponent->SetupAttachment(RootComponent);
	AimProjectorComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AimProjectorComponent->SetGenerateOverlapEvents(false);
	AimProjectorComponent->SetCastShadow(false);
	AimProjectorComponent->SetCanEverAffectNavigation(false);
	AimProjectorComponent->SetVisibility(false, true);


}

void AZhoenusPawn::BeginPlay()
{
	Super::BeginPlay();
	UNiagaraSystem* NS = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Effects/ShipEngineFlare.ShipEngineFlare"), nullptr, LOAD_None, nullptr);
	LeftThruster = UNiagaraFunctionLibrary::SpawnSystemAttached(NS, RootComponent, NAME_None, FVector(-69.7f, -31.2f, 12.f), FRotator(0.f, 0.f, 0.f), FVector(.068f, .068f, .068f), EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::None);
	RightThruster = UNiagaraFunctionLibrary::SpawnSystemAttached(NS, RootComponent, NAME_None, FVector(-69.7f, 31.2f, 12.f), FRotator(0.f, 0.f, 0.f), FVector(.068f, .068f, .068f), EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::None);
	CreateAimProjector();
	UpdateAimProjector();
}

void AZhoenusPawn::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateAimProjector();
}

void AZhoenusPawn::CreateAimProjector()
{
	if (!bEnableAimProjector || AimProjectorComponent == nullptr)
	{
		return;
	}

	UMaterialInterface* const ReticleMaterial = LoadObject<UMaterialInterface>(nullptr, *AimProjectorMaterialPath, nullptr, LOAD_None, nullptr);
	if (ReticleMaterial == nullptr)
	{
		UE_LOG(LogZhoenusPawn, Warning, TEXT("Aim projector material missing at %s."), *AimProjectorMaterialPath);
		return;
	}

	AimProjectorComponent->SetElements({});
	AimProjectorComponent->AddElement(ReticleMaterial, nullptr, false, AimProjectorScale, AimProjectorScale, nullptr);
	AimProjectorComponent->SetVisibility(false, true);
}

void AZhoenusPawn::UpdateAimProjector()
{
	if (!bEnableAimProjector || AimProjectorComponent == nullptr)
	{
		SetAimProjectorVisible(false);
		return;
	}

	const FVector FireDirection = GetProjectileFireDirection();
	const FProjectileAimTraceResult AimTrace = GetProjectileAimTrace(AimProjectorTraceDistance);
	const float VisibleDistance = FMath::Min(AimTrace.Distance, AimProjectorMaxVisibleDistance);
	const FVector AimPoint = AimTrace.SpawnLocation + FireDirection * VisibleDistance - FireDirection * AimProjectorDepthBias;

	AimProjectorComponent->SetWorldLocation(AimPoint);
	AimProjectorComponent->SetVisibility(true, true);
}

float AZhoenusPawn::GetProjectileAggroRadius() const
{
	return FMath::Max(0.f, AimProjectorScale * AimProjectorAggroRadiusScale);
}

void AZhoenusPawn::SetAimProjectorVisible(const bool bVisible)
{
	if (AimProjectorComponent != nullptr)
	{
		AimProjectorComponent->SetVisibility(bVisible, true);
	}
}
