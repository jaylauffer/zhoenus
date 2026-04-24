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
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Materials/MaterialInterface.h"
#include "EngineUtils.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundBase.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Widgets/Input/SVirtualJoystick.h"

DEFINE_LOG_CATEGORY_STATIC(LogZhoenusPawn, Log, All);

namespace
{
	const FName AimProjectorTintParameterName(TEXT("Tint"));
	const FName AimProjectorDetectedTargetParameterName(TEXT("DetectedTarget"));
}



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
	UpdateAimProjector(0.f);
}

void AZhoenusPawn::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateAimProjector(DeltaSeconds);
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

	AimProjectorMaterialInstance = UMaterialInstanceDynamic::Create(ReticleMaterial, this);
	if (AimProjectorMaterialInstance != nullptr)
	{
		AimProjectorMaterialInstance->SetVectorParameterValue(AimProjectorTintParameterName, AimProjectorIdleTint);
		AimProjectorMaterialInstance->SetScalarParameterValue(AimProjectorDetectedTargetParameterName, 0.f);
	}

	AimProjectorComponent->SetElements({});
	AimProjectorComponent->AddElement(
		AimProjectorMaterialInstance != nullptr ? AimProjectorMaterialInstance : ReticleMaterial,
		nullptr,
		false,
		AimProjectorScale,
		AimProjectorScale,
		nullptr);
	AimProjectorComponent->SetVisibility(false, true);
}

void AZhoenusPawn::UpdateAimProjector(const float DeltaSeconds)
{
	if (!bEnableAimProjector || AimProjectorComponent == nullptr)
	{
		AimProjectorDetectedTargetState = 0.f;
		SetAimProjectorVisible(false);
		return;
	}

	const FVector FireDirection = GetProjectileFireDirection();
	const FProjectileAimTraceResult VisibleAimTrace = GetProjectileAimTrace(AimProjectorTraceDistance);
	const float VisibleDistance = FMath::Min(VisibleAimTrace.Distance, AimProjectorMaxVisibleDistance);
	const FVector AimPoint = VisibleAimTrace.SpawnLocation + FireDirection * VisibleDistance - FireDirection * AimProjectorDepthBias;

	const AZapEmProjectile* const ProjectileDefaults = AZapEmProjectile::StaticClass()->GetDefaultObject<AZapEmProjectile>();
	const float ProjectileRangeTraceDistance = ProjectileDefaults != nullptr
		? ProjectileDefaults->GetConfiguredMaxTravelDistance()
		: AimProjectorTraceDistance;
	const FProjectileAimTraceResult ProjectileRangeTrace = GetProjectileAimTrace(ProjectileRangeTraceDistance);
	const float TargetPresence = GetAimProjectorAggroCueStrength(ProjectileRangeTrace);

	AimProjectorComponent->SetWorldLocation(AimPoint);
	AimProjectorComponent->SetVisibility(true, true);
	UpdateAimProjectorMaterial(DeltaSeconds, TargetPresence);
}

float AZhoenusPawn::GetProjectileAggroRadius() const
{
	return FMath::Max(0.f, AimProjectorScale * AimProjectorAggroRadiusScale);
}

float AZhoenusPawn::GetAimProjectorAggroCueStrength(const FProjectileAimTraceResult& ProjectileRangeTrace) const
{
	UWorld* const World = GetWorld();
	if (World == nullptr)
	{
		return 0.f;
	}

	const AZapEmProjectile* const ProjectileDefaults = AZapEmProjectile::StaticClass()->GetDefaultObject<AZapEmProjectile>();
	if (ProjectileDefaults == nullptr)
	{
		return 0.f;
	}

	const float ProjectileReachDistance = FMath::Max(0.f, ProjectileRangeTrace.Distance);
	const float InitialAggroRadius = GetProjectileAggroRadius();
	if (ProjectileReachDistance <= KINDA_SMALL_NUMBER || InitialAggroRadius <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}

	const FVector FireDirection = GetProjectileFireDirection();
	const FVector SegmentStart = ProjectileRangeTrace.SpawnLocation;
	const FVector SegmentEnd = SegmentStart + FireDirection * ProjectileReachDistance;
	float StrongestPresence = 0.f;

	for (TActorIterator<ADonutFlyerPawn> DonutIt(World); DonutIt; ++DonutIt)
	{
		ADonutFlyerPawn* const Donut = *DonutIt;
		if (!IsValid(Donut))
		{
			continue;
		}

		FVector DonutOrigin = FVector::ZeroVector;
		FVector DonutExtent = FVector::ZeroVector;
		Donut->GetActorBounds(false, DonutOrigin, DonutExtent);

		float DonutRadius = Donut->GetSimpleCollisionRadius();
		if (DonutRadius <= KINDA_SMALL_NUMBER)
		{
			DonutRadius = DonutExtent.GetMax();
		}

		const float ForwardDistance = FVector::DotProduct(DonutOrigin - SegmentStart, FireDirection);
		if (ForwardDistance < -DonutRadius || ForwardDistance > ProjectileReachDistance + DonutRadius)
		{
			continue;
		}

		const float ProjectileTravelDistance = FMath::Clamp(ForwardDistance, 0.f, ProjectileReachDistance);
		const FVector ClosestPoint = FMath::ClosestPointOnSegment(DonutOrigin, SegmentStart, SegmentEnd);
		const float AggroRadiusAtClosestPoint = ProjectileDefaults->GetAggroRadiusAtTravelDistance(
			InitialAggroRadius,
			ProjectileTravelDistance);
		const float EffectiveRadius = AggroRadiusAtClosestPoint + DonutRadius;
		const float DistanceToSegment = FVector::Dist(DonutOrigin, ClosestPoint);
		if (EffectiveRadius <= KINDA_SMALL_NUMBER || DistanceToSegment > EffectiveRadius)
		{
			continue;
		}

		const float Presence = 1.f - FMath::Clamp(DistanceToSegment / EffectiveRadius, 0.f, 1.f);
		StrongestPresence = FMath::Max(StrongestPresence, Presence);
	}

	return StrongestPresence;
}

void AZhoenusPawn::UpdateAimProjectorMaterial(const float DeltaSeconds, const float TargetPresence)
{
	if (AimProjectorMaterialInstance == nullptr)
	{
		return;
	}

	const float BlendSpeed = FMath::Max(0.f, AimProjectorDetectedTargetBlendSpeed);
	AimProjectorDetectedTargetState = BlendSpeed > 0.f
		? FMath::FInterpTo(AimProjectorDetectedTargetState, TargetPresence, DeltaSeconds, BlendSpeed)
		: TargetPresence;

	const FLinearColor ActiveTint = FLinearColor::LerpUsingHSV(
		AimProjectorIdleTint,
		AimProjectorDetectedTargetTint,
		AimProjectorDetectedTargetState);

	AimProjectorMaterialInstance->SetVectorParameterValue(AimProjectorTintParameterName, ActiveTint);
	AimProjectorMaterialInstance->SetScalarParameterValue(AimProjectorDetectedTargetParameterName, AimProjectorDetectedTargetState);
}

void AZhoenusPawn::SetAimProjectorVisible(const bool bVisible)
{
	if (AimProjectorComponent != nullptr)
	{
		AimProjectorComponent->SetVisibility(bVisible, true);
	}
}
