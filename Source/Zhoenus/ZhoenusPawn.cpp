// Copyright 2025 Run Rong Games, All Rights Reserved.

#include "ZhoenusPawn.h"
#include "ZapEmProjectile.h"
#include "DonutFlyerPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Components/SpotLightComponent.h"
#include "NiagaraCommon.h"
#include "NiagaraComponent.h"
#include "NiagaraComponentSettings.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundBase.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Widgets/Input/SVirtualJoystick.h"

DEFINE_LOG_CATEGORY_STATIC(LogZhoenusPawn, Log, All);



AZhoenusPawn::AZhoenusPawn(const FObjectInitializer &initializer) : Super(initializer)
{
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


	bReplicates = true;
}

void AZhoenusPawn::BeginPlay()
{
	Super::BeginPlay();
	UNiagaraSystem* NS = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Effects/ShipEngineFlare.ShipEngineFlare"), nullptr, LOAD_None, nullptr);
	LeftThruster = UNiagaraFunctionLibrary::SpawnSystemAttached(NS, RootComponent, NAME_None, FVector(-69.7f, -31.2f, 12.f), FRotator(0.f, 0.f, 0.f), FVector(.068f, .068f, .068f), EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::None);
	RightThruster = UNiagaraFunctionLibrary::SpawnSystemAttached(NS, RootComponent, NAME_None, FVector(-69.7f, 31.2f, 12.f), FRotator(0.f, 0.f, 0.f), FVector(.068f, .068f, .068f), EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::None);
}
