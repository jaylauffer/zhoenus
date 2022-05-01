// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZhoenusMovementComponent.h"
#include "ZhoenusMovementSimulation.h"
#include "NetworkPredictionProxyInit.h"
#include "NetworkPredictionModelDefRegistry.h"
#include "NetworkPredictionProxyWrite.h"

#include "GameFramework/CharacterMovementComponent.h" // To facilitate A/B testing by disabling CMC if present
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogZhoenusMovement, Log, All);

namespace ZhoenusMovementCVars
{
static float MaxSpeed = 1200.f;
static FAutoConsoleVariableRef CVarMaxSpeed(TEXT("motion.MaxSpeed"),
	MaxSpeed,
	TEXT("Temp value for testing changes to max speed."),
	ECVF_Default);

static int32 RequestMispredict = 0;
static FAutoConsoleVariableRef CVarRequestMispredict(TEXT("fp.RequestMispredict"),
	RequestMispredict, TEXT("Causes a misprediction by inserting random value into stream on authority side"), ECVF_Default);
}

float UZhoenusMovementComponent::GetDefaultMaxSpeed() { return ZhoenusMovementCVars::MaxSpeed; }

// ----------------------------------------------------------------------------------------------------------
//	FZhoenusMovementModelDef: the piece that ties everything together that we use to register with the NP system.
// ----------------------------------------------------------------------------------------------------------

class FZhoenusMovementModelDef : public FNetworkPredictionModelDef
{
public:

	NP_MODEL_BODY();

	using Simulation = FZhoenusMovementSimulation;
	using StateTypes = ZhoenusMovementStateTypes;
	using Driver = UZhoenusMovementComponent;

	static const TCHAR* GetName() { return TEXT("ZhoenusMovement"); }
	static constexpr int32 GetSortPriority() { return (int32)ENetworkPredictionSortPriority::PreKinematicMovers; }
};

NP_MODEL_REGISTER(FZhoenusMovementModelDef);

// ----------------------------------------------------------------------------------------------------------
//	UZhoenusMovementComponent
// ----------------------------------------------------------------------------------------------------------

UZhoenusMovementComponent::UZhoenusMovementComponent()
{

}

void UZhoenusMovementComponent::InitializeNetworkPredictionProxy()
{
	OwnedMovementSimulation = MakePimpl<FZhoenusMovementSimulation>();
	InitZhoenusMovementSimulation(OwnedMovementSimulation.Get());
	
	NetworkPredictionProxy.Init<FZhoenusMovementModelDef>(GetWorld(), GetReplicationProxies(), OwnedMovementSimulation.Get(), this);
}

void UZhoenusMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// TEMP! Disable existing CMC if it is activate. Just makes A/B testing easier for now.
	if (AActor* Owner = GetOwner())
	{
		if (UCharacterMovementComponent* OldComp = Owner->FindComponentByClass<UCharacterMovementComponent>())
		{
			if (OldComp->IsActive())
			{
				OldComp->Deactivate();
			}
		}

		Owner->SetReplicatingMovement(false);
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	const ENetRole OwnerRole = GetOwnerRole();

	// Check if we should trip a mispredict. (Note how its not possible to do this inside the Update function!)
	if (OwnerRole == ROLE_Authority && ZhoenusMovementCVars::RequestMispredict)
	{
		FZhoenusMovementSimulation::ForceMispredict = true;
		ZhoenusMovementCVars::RequestMispredict = 0;
	}


	static bool bDrawSimLocation = true;
	static bool bDrawPresentationLocation = true;
	/*
	if (bDrawSimLocation)
	{
		DrawDebugSphere(GetWorld(), 
		NetworkPredictionProxy.ReadSyncState<FFlyingMovementSyncState>(ENetworkPredictionStateRead::Simulation)->Location + FVector(0.f, 0.f, 100.f),
		25.f, 12, FColor::Green, false, -1.f);
	}
	if (bDrawPresentationLocation)
	{
		DrawDebugSphere(GetWorld(), 
			NetworkPredictionProxy.ReadSyncState<FFlyingMovementSyncState>()->Location + FVector(0.f, 0.f, 200.f),
			25.f, 12, FColor::Red, false, -1.f);
	}
	*/
	// Temp
	/*
	if (OwnerRole == ROLE_Authority)
	{
		if (MovementAuxState->Get()->MaxSpeed != FlyingMovementCVars::MaxSpeed)
		{
			MovementAuxState->Modify([](FFlyingMovementAuxState& Aux)
			{
				Aux.MaxSpeed = FlyingMovementCVars::MaxSpeed;
			});
		}
	}
	*/
}

void UZhoenusMovementComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ActiveMovementSimulation)
	{
		ActiveMovementSimulation->OnBeginOverlap(OverlappedComp, Other, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	}
}

void UZhoenusMovementComponent::ProduceInput(const int32 DeltaTimeMS, FZhoenusMovementInputCmd* Cmd)
{
	// This isn't ideal. It probably makes sense for the component to do all the input binding rather.
	ProduceInputDelegate.ExecuteIfBound(DeltaTimeMS, *Cmd);
}

void UZhoenusMovementComponent::RestoreFrame(const FZhoenusMovementSyncState* SyncState, const FZhoenusMovementAuxState* AuxState)
{
	FTransform Transform(SyncState->Rotation, SyncState->Location, UpdatedComponent->GetComponentTransform().GetScale3D() );
	UpdatedComponent->SetRelativeTransform(Transform, false, nullptr, ETeleportType::TeleportPhysics);
	UpdatedComponent->ComponentVelocity = SyncState->Velocity;
}

void UZhoenusMovementComponent::FinalizeFrame(const FZhoenusMovementSyncState* SyncState, const FZhoenusMovementAuxState* AuxState)
{
	// The component will often be in the "right place" already on FinalizeFrame, so a comparison check makes sense before setting it.
	if (UpdatedComponent->GetComponentLocation().Equals(SyncState->Location) == false || UpdatedComponent->GetComponentQuat().Equals(SyncState->Rotation, FZhoenusMovementSimulation::ROTATOR_TOLERANCE) == false)
	{
		RestoreFrame(SyncState, AuxState);
	}
}

void UZhoenusMovementComponent::InitializeSimulationState(FZhoenusMovementSyncState* Sync, FZhoenusMovementAuxState* Aux)
{
	npCheckSlow(UpdatedComponent);
	npCheckSlow(Sync);
	npCheckSlow(Aux);

	Sync->Location = UpdatedComponent->GetComponentLocation();
	Sync->Rotation = UpdatedComponent->GetComponentQuat();

	Aux->MaxSpeed = GetDefaultMaxSpeed();
}

// Init function. This is broken up from ::InstantiateNetworkedSimulation and templated so that subclasses can share the init code
void UZhoenusMovementComponent::InitZhoenusMovementSimulation(FZhoenusMovementSimulation* Simulation)
{
	check(UpdatedComponent);
	check(ActiveMovementSimulation == nullptr); // Reinstantiation not supported
	ActiveMovementSimulation = Simulation;

	Simulation->SetComponents(UpdatedComponent, UpdatedPrimitive);
}

float UZhoenusMovementComponent::GetMaxMoveSpeed() const
{
	if (const FZhoenusMovementAuxState* AuxState = NetworkPredictionProxy.ReadAuxState<FZhoenusMovementAuxState>())
	{
		return AuxState->MaxSpeed;
	}
	return 0;
}

void UZhoenusMovementComponent::SetMaxMoveSpeed(float NewMaxMoveSpeed)
{
	NetworkPredictionProxy.WriteAuxState<FZhoenusMovementAuxState>([NewMaxMoveSpeed](FZhoenusMovementAuxState& AuxState)
	{
		AuxState.MaxSpeed = NewMaxMoveSpeed;
	}, "SetMaxMoveSpeed");
}

void UZhoenusMovementComponent::AddMaxMoveSpeed(float AdditiveMaxMoveSpeed)
{
	NetworkPredictionProxy.WriteAuxState<FZhoenusMovementAuxState>([AdditiveMaxMoveSpeed](FZhoenusMovementAuxState& AuxState)
	{
		AuxState.MaxSpeed += AdditiveMaxMoveSpeed;
	}, "AddMaxMoveSpeed");
}