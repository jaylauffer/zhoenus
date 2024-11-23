// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

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


//UNiagaraComponent* CreateNiagaraSystem(UNiagaraSystem* SystemTemplate, UWorld* World, AActor* Actor, const FName &Name, bool bAutoDestroy, ENCPoolMethod PoolingMethod)
//{
//	UNiagaraComponent* NiagaraComponent = nullptr;
//
//	if (FApp::CanEverRender() && World && !World->IsNetMode(NM_DedicatedServer))
//	{
//		if (PoolingMethod == ENCPoolMethod::None)
//		{
//			if (UNiagaraComponentSettings::ShouldForceAutoPooling(SystemTemplate))
//			{
//				if (FNiagaraWorldManager* WorldManager = FNiagaraWorldManager::Get(World))
//				{
//					UNiagaraComponentPool* ComponentPool = WorldManager->GetComponentPool();
//					NiagaraComponent = ComponentPool->CreateWorldParticleSystem(SystemTemplate, World, ENCPoolMethod::AutoRelease);
//				}
//			}
//			else
//			{
//				NiagaraComponent = Actor->CreateDefaultSubobject<UNiagaraComponent>(Name);
//				NiagaraComponent->SetAsset(SystemTemplate);
//				NiagaraComponent->bAutoActivate = false;
//				NiagaraComponent->SetAutoDestroy(bAutoDestroy);
//				NiagaraComponent->bAllowAnyoneToDestroyMe = true;
//				NiagaraComponent->SetVisibleInRayTracing(false);
//			}
//		}
//		else if (FNiagaraWorldManager* WorldManager = FNiagaraWorldManager::Get(World))
//		{
//			UNiagaraComponentPool* ComponentPool = WorldManager->GetComponentPool();
//			NiagaraComponent = ComponentPool->CreateWorldParticleSystem(SystemTemplate, World, PoolingMethod);
//		}
//	}
//	return NiagaraComponent;
//}
//
///**
//* Spawns a Niagara System attached to a component
//* @return			The spawned UNiagaraComponent
//*/
//
//UNiagaraComponent* SpawnNiagaraSystemAttachedWithParams(FFXSystemSpawnParameters& SpawnParams)
//{
//	LLM_SCOPE(ELLMTag::Niagara);
//
//	UNiagaraComponent* PSC = nullptr;
//	UNiagaraSystem* NiagaraSystem = Cast<UNiagaraSystem>(SpawnParams.SystemTemplate);
//	if (NiagaraSystem)
//	{
//		if (!SpawnParams.AttachToComponent)
//		{
//			UE_LOG(LogScript, Warning, TEXT("UGameplayStatics::SpawnNiagaraEmitterAttached: NULL AttachComponent specified!"));
//		}
//		else
//		{
//			UWorld* const World = SpawnParams.AttachToComponent->GetWorld();
//			if (World && !World->IsNetMode(NM_DedicatedServer))
//			{
//				bool bShouldCull = false;
//				if (SpawnParams.bPreCullCheck)
//				{
//					//Don't precull if this is a local player linked effect and the system doesn't allow us to cull those.
//					bool bIsPlayerEffect = SpawnParams.bIsPlayerEffect || FNiagaraWorldManager::IsComponentLocalPlayerLinked(SpawnParams.AttachToComponent);
//					if (NiagaraSystem->AllowCullingForLocalPlayers() || bIsPlayerEffect == false)
//					{
//						FNiagaraWorldManager* WorldManager = FNiagaraWorldManager::Get(World);
//						//TODO: For now using the attach parent location and ignoring the emitters relative location which is clearly going to be a bit wrong in some cases.
//						bShouldCull = WorldManager->ShouldPreCull(NiagaraSystem, SpawnParams.AttachToComponent->GetComponentLocation());
//					}
//				}
//
//				if (!bShouldCull)
//				{
//					PSC = CreateNiagaraSystem(NiagaraSystem, World, SpawnParams.AttachToComponent->GetOwner(), SpawnParams.AttachPointName, SpawnParams.bAutoDestroy, ToNiagaraPooling(SpawnParams.PoolingMethod));
//					if (PSC)
//					{
//						if (SpawnParams.bIsPlayerEffect)
//						{
//							PSC->SetForceLocalPlayerEffect(true);
//						}
//
////#if WITH_EDITOR
////						if (GForceNiagaraSpawnAttachedSolo > 0)
////						{
////							PSC->SetForceSolo(true);
////						}
////#endif
//						auto SetupRelativeTransforms = [&]()
//						{
//							if (SpawnParams.LocationType == EAttachLocation::KeepWorldPosition)
//							{
//								const FTransform ParentToWorld = SpawnParams.AttachToComponent->GetSocketTransform(SpawnParams.AttachPointName);
//								const FTransform ComponentToWorld(SpawnParams.Rotation, SpawnParams.Location, SpawnParams.Scale);
//								const FTransform RelativeTM = ComponentToWorld.GetRelativeTransform(ParentToWorld);
//								PSC->SetRelativeLocation_Direct(RelativeTM.GetLocation());
//								PSC->SetRelativeRotation_Direct(RelativeTM.GetRotation().Rotator());
//								PSC->SetRelativeScale3D_Direct(RelativeTM.GetScale3D());
//							}
//							else
//							{
//								PSC->SetRelativeLocation_Direct(SpawnParams.Location);
//								PSC->SetRelativeRotation_Direct(SpawnParams.Rotation);
//
//								if (SpawnParams.LocationType == EAttachLocation::SnapToTarget)
//								{
//									// SnapToTarget indicates we "keep world scale", this indicates we we want the inverse of the parent-to-world scale 
//									// to calculate world scale at Scale 1, and then apply the passed in Scale
//									const FTransform ParentToWorld = SpawnParams.AttachToComponent->GetSocketTransform(SpawnParams.AttachPointName);
//									PSC->SetRelativeScale3D_Direct(SpawnParams.Scale * ParentToWorld.GetSafeScaleReciprocal(ParentToWorld.GetScale3D()));
//								}
//								else
//								{
//									PSC->SetRelativeScale3D_Direct(SpawnParams.Scale);
//								}
//							}
//						};
//
//						if (PSC->IsRegistered())
//						{
//							//It is now possible for us to be already regisetered so we must use AttachToComponent() instead.
//							SetupRelativeTransforms();
//							PSC->AttachToComponent(SpawnParams.AttachToComponent, FAttachmentTransformRules::KeepRelativeTransform, SpawnParams.AttachPointName);
//						}
//						else
//						{
//							PSC->SetupAttachment(SpawnParams.AttachToComponent, SpawnParams.AttachPointName);
//							SetupRelativeTransforms();
//							PSC->RegisterComponentWithWorld(World);
//						}
//
//						if (SpawnParams.bAutoActivate)
//						{
//							PSC->Activate(true);
//						}
//
//						// Notify the texture streamer so that PSC gets managed as a dynamic component.
//						IStreamingManager::Get().NotifyPrimitiveUpdated(PSC);
//					}
//				}
//			}
//		}
//	}
//	return PSC;
//}
//
//UNiagaraComponent* SpawnNiagaraSystemAttached(
//	UNiagaraSystem* SystemTemplate,
//	USceneComponent* AttachToComponent,
//	FName AttachPointName,
//	FVector Location,
//	FRotator Rotation,
//	FVector Scale,
//	EAttachLocation::Type LocationType,
//	bool bAutoDestroy,
//	ENCPoolMethod PoolingMethod,
//	bool bAutoActivate = true,
//	bool bPreCullCheck = true
//)
//{
//	FFXSystemSpawnParameters SpawnParams;
//	SpawnParams.SystemTemplate = SystemTemplate;
//	SpawnParams.AttachToComponent = AttachToComponent;
//	SpawnParams.AttachPointName = AttachPointName;
//	SpawnParams.Location = Location;
//	SpawnParams.Rotation = Rotation;
//	SpawnParams.Scale = Scale;
//	SpawnParams.LocationType = LocationType;
//	SpawnParams.bAutoDestroy = bAutoDestroy;
//	SpawnParams.bAutoActivate = bAutoActivate;
//	SpawnParams.PoolingMethod = ToPSCPoolMethod(PoolingMethod);
//	SpawnParams.bPreCullCheck = bPreCullCheck;
//	return SpawnNiagaraSystemAttachedWithParams(SpawnParams);
//}

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

	//const FName NAME_LeftThruster{ TEXT("LeftThruster") };
	//LeftThruster = SpawnNiagaraSystemAttached(Statics.ThrusterParticles.Object, RootComponent, NAME_LeftThruster, FVector(-69.7f, -31.2f, 12.f), FRotator(0.f, 0.f, 0.f), FVector(.068f, .068f, .068f), EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::None);
	//LeftThruster = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LeftThruster"));
	//LeftThruster->SetTemplate(Statics.ThrusterParticles.Object);
	//LeftThruster->SetupAttachment(RootComponent);
	//LeftThruster->SetRelativeLocation(FVector(-69.7f, -31.2f, 12.f));
	//LeftThruster->SetRelativeScale3D(FVector(.068f, .068f, .068f));
	//LeftThruster->SetLightColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("a5a5ffa5"))));

	//const FName NAME_RightThruster{ TEXT("RightThruster") };
	//RightThruster = SpawnNiagaraSystemAttached(Statics.ThrusterParticles.Object, RootComponent, NAME_RightThruster, FVector(-69.7f, 31.2f, 12.f), FRotator(0.f, 0.f, 0.f), FVector(.068f, .068f, .068f), EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::None);

	//RightThruster = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RightThruster"));
	//RightThruster->SetTemplate(Statics.ThrusterParticles.Object);
	//RightThruster->SetupAttachment(RootComponent);
	//RightThruster->SetRelativeLocation(FVector(-69.7f, 31.2f, 12.f));
	//RightThruster->SetRelativeScale3D(FVector(.068f, .068f, .068f));
	////RightThruster->SetLightColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("a5a5ffa5"))));

	bReplicates = true;
}

void AZhoenusPawn::BeginPlay()
{
	Super::BeginPlay();
	UNiagaraSystem* NS = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Effects/ShipEngineFlare.ShipEngineFlare"), nullptr, LOAD_None, nullptr);
	LeftThruster = UNiagaraFunctionLibrary::SpawnSystemAttached(NS, RootComponent, NAME_None, FVector(-69.7f, -31.2f, 12.f), FRotator(0.f, 0.f, 0.f), FVector(.068f, .068f, .068f), EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::None);
	RightThruster = UNiagaraFunctionLibrary::SpawnSystemAttached(NS, RootComponent, NAME_None, FVector(-69.7f, 31.2f, 12.f), FRotator(0.f, 0.f, 0.f), FVector(.068f, .068f, .068f), EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::None);
}
