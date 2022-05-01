// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Misc/StringBuilder.h"
#include "BaseMovementSimulation.h"
#include "NetworkPredictionReplicationProxy.h"
#include "NetworkPredictionStateTypes.h"
#include "NetworkPredictionTickState.h"
#include "NetworkPredictionSimulation.h"

// -------------------------------------------------------------------------------------------------------------------------------
// FlyingMovement: simple flying movement that was based on UE4's FloatingPawnMovement
// -------------------------------------------------------------------------------------------------------------------------------

// State the client generates
struct FZhoenusMovementInputCmd
{
	// Input: "pure" input for this frame. At this level, frame time has not been accounted for. (E.g., "move straight" would be (1,0,0) regardless of frame time)
	FQuat RotationInput;
	FVector MovementInput;

	FZhoenusMovementInputCmd()
		: RotationInput(ForceInitToZero)
		, MovementInput(ForceInitToZero)
	{ }

	void NetSerialize(const FNetSerializeParams& P)
	{
		P.Ar << RotationInput;
		P.Ar << MovementInput;
	}

	void ToString(FAnsiStringBuilderBase& Out) const
	{
		Out.Appendf("MovementInput: Thrust=%.2f AutoCorrect=%.2f, RateOfFire=%.2f\n", RotationInput.W, MovementInput.X, MovementInput.Y);
		Out.Appendf("RotationInput: Pitch=%.2f Yaw=%.2f Roll=%.2f\n", RotationInput.X, RotationInput.Y, RotationInput.Z);
	}
};

// State we are evolving frame to frame and keeping in sync
struct FZhoenusMovementSyncState
{
	FVector Location;
	FVector Velocity;
	FQuat Rotation;
	float CurrentRollSpeed;
	float CurrentYawSpeed;
	float CurrentPitchSpeed;
	float CurrentForwardSpeed;
	float AutoCorrectRate;


	FZhoenusMovementSyncState()
		: Location(ForceInitToZero)
		, Velocity(ForceInitToZero)
		, Rotation(ForceInitToZero)
		, CurrentRollSpeed{0.f}
		, CurrentYawSpeed{0.f}
		, CurrentPitchSpeed{0.f}
		, CurrentForwardSpeed{0.f}
		, AutoCorrectRate{0.f}
	{ }

	bool ShouldReconcile(const FZhoenusMovementSyncState& AuthorityState) const;

	void NetSerialize(const FNetSerializeParams& P)
	{
		P.Ar << Location;
		P.Ar << Velocity;
		P.Ar << Rotation;
		P.Ar << CurrentRollSpeed;
		P.Ar << CurrentYawSpeed;
		P.Ar << CurrentPitchSpeed;
		P.Ar << CurrentForwardSpeed;
		P.Ar << AutoCorrectRate;
	}

	void ToString(FAnsiStringBuilderBase& Out) const
	{
		FRotator Rotator{ Rotation.Rotator() };
		Out.Appendf("Loc: X=%.2f Y=%.2f Z=%.2f\n", Location.X, Location.Y, Location.Z);
		Out.Appendf("Vel: X=%.2f Y=%.2f Z=%.2f\n", Velocity.X, Velocity.Y, Velocity.Z);
		Out.Appendf("Rot: P=%.2f Y=%.2f R=%.2f\n", Rotator.Pitch, Rotator.Yaw, Rotator.Roll);
		Out.Appendf("Forward: %.2f Pitch: %.2f Yaw: %.2f Roll: %.2f", CurrentForwardSpeed, CurrentPitchSpeed, CurrentYawSpeed, CurrentRollSpeed);
	}

	void Interpolate(const FZhoenusMovementSyncState* From, const FZhoenusMovementSyncState* To, float PCT)
	{
		static constexpr float TeleportThreshold = 1000.f * 1000.f;
		if (FVector::DistSquared(From->Location, To->Location) > TeleportThreshold)
		{
			*this = *To;
		}
		else
		{
			Location = FMath::Lerp(From->Location, To->Location, PCT);
			Velocity = FMath::Lerp(From->Velocity, To->Velocity, PCT);
			Rotation = FMath::Lerp(From->Rotation, To->Rotation, PCT);
			CurrentRollSpeed = FMath::Lerp(From->CurrentRollSpeed, To->CurrentRollSpeed, PCT);
			CurrentYawSpeed = FMath::Lerp(From->CurrentYawSpeed, To->CurrentYawSpeed, PCT);
			CurrentPitchSpeed = FMath::Lerp(From->CurrentPitchSpeed, To->CurrentPitchSpeed, PCT);
			CurrentForwardSpeed = FMath::Lerp(From->CurrentForwardSpeed, To->CurrentForwardSpeed, PCT);
			AutoCorrectRate = FMath::Lerp(From->AutoCorrectRate, To->AutoCorrectRate, PCT);
		}
	}
};

// Auxiliary state that is input into the simulation.
struct FZhoenusMovementAuxState
{	
	float MaxSpeed = 3000.f;
	float MinSpeed = -1000.f;
	float Acceleration = 500.f;
	float TurnSpeed = 50.f;
	float RollSpeed = 100.f;


	bool ShouldReconcile(const FZhoenusMovementAuxState& AuthorityState) const;

	void NetSerialize(const FNetSerializeParams& P)
	{
		P.Ar << MaxSpeed;
		P.Ar << MinSpeed;
		P.Ar << Acceleration;
		P.Ar << TurnSpeed;
		P.Ar << RollSpeed;
	}

	void ToString(FAnsiStringBuilderBase& Out) const
	{
		Out.Appendf("MaxSpeed: %.2f\n", MaxSpeed);
		Out.Appendf("MinSpeed: %.2f\n", MinSpeed);
		Out.Appendf("Acceleration: %.2f\n", Acceleration);
		Out.Appendf("TurnSpeed: %.2f\n", TurnSpeed);
		Out.Appendf("RollSpeed: %.2f\n", RollSpeed);
	}

	void Interpolate(const FZhoenusMovementAuxState* From, const FZhoenusMovementAuxState* To, float PCT)
	{
		// This is probably a good case where interpolating values is pointless and it could just snap
		// to the 'To' state.
		MaxSpeed = FMath::Lerp(From->MaxSpeed, To->MaxSpeed, PCT);
		MinSpeed = FMath::Lerp(From->MinSpeed, To->MinSpeed, PCT);
		Acceleration = FMath::Lerp(From->Acceleration, To->Acceleration, PCT);
		TurnSpeed = FMath::Lerp(From->TurnSpeed, To->TurnSpeed, PCT);
		RollSpeed = FMath::Lerp(From->RollSpeed, To->RollSpeed, PCT);
	}
};

using ZhoenusMovementStateTypes = TNetworkPredictionStateTypes<FZhoenusMovementInputCmd, FZhoenusMovementSyncState, FZhoenusMovementAuxState>;

class FZhoenusMovementSimulation : public FBaseMovementSimulation
{
public:

	/** Main update function */
	void SimulationTick(const FNetSimTimeStep& TimeStep, const TNetSimInput<ZhoenusMovementStateTypes>& Input, const TNetSimOutput<ZhoenusMovementStateTypes>& Output);

	// Callbacks
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// general tolerance value for rotation checks
	static constexpr float ROTATOR_TOLERANCE = (1e-3);

	/** Dev tool to force simple mispredict */
	static bool ForceMispredict;

protected:

	float SlideAlongSurface(const FVector& Delta, float Time, const FQuat Rotation, const FVector& Normal, FHitResult& Hit, bool bHandleImpact);
};