// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "BaseMovementComponent.h"
#include "Templates/PimplPtr.h"
#include "ZhoenusMovementComponent.generated.h"

struct FZhoenusMovementInputCmd;
struct FZhoenusMovementSyncState;
struct FZhoenusMovementAuxState;

class FZhoenusMovementSimulation;

// -------------------------------------------------------------------------------------------------------------------------------
// ActorComponent for running FlyingMovement 
// -------------------------------------------------------------------------------------------------------------------------------

UCLASS(BlueprintType, meta=(BlueprintSpawnableComponent))
class UZhoenusMovementComponent : public UBaseMovementComponent
{
	GENERATED_BODY()

public:

	UZhoenusMovementComponent();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	// Forward input producing event to someone else (probably the owning actor)
	DECLARE_DELEGATE_TwoParams(FProduceFlyingInput, const int32 /*SimTime*/, FZhoenusMovementInputCmd& /*Cmd*/)
	FProduceFlyingInput ProduceInputDelegate;

	// BeginOverlap has to be bound to a ufunction, so we have no choice but to bind here and forward into simulation code. Not ideal.
	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// --------------------------------------------------------------------------------
	// NP Driver
	// --------------------------------------------------------------------------------

	// Get latest local input prior to simulation step
	void ProduceInput(const int32 DeltaTimeMS, FZhoenusMovementInputCmd* Cmd);

	// Restore a previous frame prior to resimulating
	void RestoreFrame(const FZhoenusMovementSyncState* SyncState, const FZhoenusMovementAuxState* AuxState);

	// Take output for simulation
	void FinalizeFrame(const FZhoenusMovementSyncState* SyncState, const FZhoenusMovementAuxState* AuxState);

	// Seed initial values based on component's state
	void InitializeSimulationState(FZhoenusMovementSyncState* Sync, FZhoenusMovementAuxState* Aux);

	float GetMaxMoveSpeed() const;
	void SetMaxMoveSpeed(float NewMaxMoveSpeed);
	void AddMaxMoveSpeed(float AdditiveMaxMoveSpeed);

protected:

	// Network Prediction
	virtual void InitializeNetworkPredictionProxy() override;
	TPimplPtr<FZhoenusMovementSimulation> OwnedMovementSimulation; // If we instantiate the sim in InitializeNetworkPredictionProxy, its stored here
	FZhoenusMovementSimulation* ActiveMovementSimulation = nullptr; // The sim driving us, set in InitFlyingMovementSimulation. Could be child class that implements InitializeNetworkPredictionProxy.

	void InitZhoenusMovementSimulation(FZhoenusMovementSimulation* Simulation);

	static float GetDefaultMaxSpeed();
};