// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZhoenusMovementSimulation.h"

#include "Containers/StringFwd.h"

DEFINE_LOG_CATEGORY_STATIC(LogZhoenusPawnSimulation, Log, All);

namespace ZhoenusPawnSimCVars
{
static float ErrorTolerance = 10.f;
static FAutoConsoleVariableRef CVarErrorTolerance(TEXT("fp.ErrorTolerance"),
	ErrorTolerance, TEXT("Location tolerance for reconcile"), ECVF_Default);
}

// -------------------------------------------------------------------------------------------------------

bool FZhoenusMovementAuxState::ShouldReconcile(const FZhoenusMovementAuxState& AuthorityState) const
{
	return false;
}

bool FZhoenusMovementSyncState::ShouldReconcile(const FZhoenusMovementSyncState& AuthorityState) const
{
	const float ErrorTolerance = ZhoenusPawnSimCVars::ErrorTolerance;
	return !AuthorityState.Location.Equals(Location, ErrorTolerance);
}

// -------------------------------------------------------------------------------------------------------

bool FZhoenusMovementSimulation::ForceMispredict = false;
static FVector ForceMispredictVelocityMagnitude = FVector(2000.f, 0.f, 0.f);

static bool IsExceedingMaxSpeed(const FVector& Velocity, float InMaxSpeed)
{
	InMaxSpeed = FMath::Max(0.f, InMaxSpeed);
	const float MaxSpeedSquared = FMath::Square(InMaxSpeed);
	
	// Allow 1% error tolerance, to account for numeric imprecision.
	const float OverVelocityPercent = 1.01f;
	return (Velocity.SizeSquared() > MaxSpeedSquared * OverVelocityPercent);
}

static FVector ComputeSlideVector(const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit)
{
	return FVector::VectorPlaneProject(Delta, Normal) * Time;
}

static void TwoWallAdjust(FVector& OutDelta, const FHitResult& Hit, const FVector& OldHitNormal)
{
	FVector Delta = OutDelta;
	const FVector HitNormal = Hit.Normal;

	if ((OldHitNormal | HitNormal) <= 0.f) //90 or less corner, so use cross product for direction
	{
		const FVector DesiredDir = Delta;
		FVector NewDir = (HitNormal ^ OldHitNormal);
		NewDir = NewDir.GetSafeNormal();
		Delta = (Delta | NewDir) * (1.f - Hit.Time) * NewDir;
		if ((DesiredDir | Delta) < 0.f)
		{
			Delta = -1.f * Delta;
		}
	}
	else //adjust to new wall
	{
		const FVector DesiredDir = Delta;
		Delta = ComputeSlideVector(Delta, 1.f - Hit.Time, HitNormal, Hit);
		if ((Delta | DesiredDir) <= 0.f)
		{
			Delta = FVector::ZeroVector;
		}
		else if ( FMath::Abs((HitNormal | OldHitNormal) - 1.f) < KINDA_SMALL_NUMBER )
		{
			// we hit the same wall again even after adjusting to move along it the first time
			// nudge away from it (this can happen due to precision issues)
			Delta += HitNormal * 0.01f;
		}
	}

	OutDelta = Delta;
}

float FZhoenusMovementSimulation::SlideAlongSurface(const FVector& Delta, float Time, const FQuat Rotation, const FVector& Normal, FHitResult& Hit, bool bHandleImpact)
{
	if (!Hit.bBlockingHit)
	{
		return 0.f;
	}

	float PercentTimeApplied = 0.f;
	const FVector OldHitNormal = Normal;

	FVector SlideDelta = ComputeSlideVector(Delta, Time, Normal, Hit);

	if ((SlideDelta | Delta) > 0.f)
	{
		SafeMoveUpdatedComponent(SlideDelta, Rotation, true, Hit, ETeleportType::None);

		const float FirstHitPercent = Hit.Time;
		PercentTimeApplied = FirstHitPercent;
		if (Hit.IsValidBlockingHit())
		{
			// Notify first impact
			if (bHandleImpact)
			{
				// !HandleImpact(Hit, FirstHitPercent * Time, SlideDelta);
			}

			// Compute new slide normal when hitting multiple surfaces.
			TwoWallAdjust(SlideDelta, Hit, OldHitNormal);

			// Only proceed if the new direction is of significant length and not in reverse of original attempted move.
			if (!SlideDelta.IsNearlyZero(1e-3f) && (SlideDelta | Delta) > 0.f)
			{
				// Perform second move
				SafeMoveUpdatedComponent(SlideDelta, Rotation, true, Hit, ETeleportType::None);
				const float SecondHitPercent = Hit.Time * (1.f - FirstHitPercent);
				PercentTimeApplied += SecondHitPercent;

				// Notify second impact
				if (bHandleImpact && Hit.bBlockingHit)
				{
					// !HandleImpact(Hit, SecondHitPercent * Time, SlideDelta);
				}
			}
		}

		return FMath::Clamp(PercentTimeApplied, 0.f, 1.f);
	}

	return 0.f;
}

namespace
{
	inline float CalcYawVelocity(float Val, const float CurrentSpeed, const float Accel, const float DeltaSeconds)
	{
		const bool bIsTurning = FMath::Abs(Val) > 0.2f;
		// If not turning, roll to reverse current roll value.
		float TargetSpeed = bIsTurning ? (Val * Accel) : 0.f;

		// Smoothly interpolate to target yaw speed
		return FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaSeconds, 2.f);
	}

	inline float CalcTurnVelocity(float Val, const float CurrentSpeed, const float CurrentVal, const float Accel, const float AutoCorrect, const float DeltaSeconds)
	{
		const bool bIsTurning = FMath::Abs(Val) > 0.2f;
		// If not turning, roll to reverse current roll value.
		float TargetSpeed = bIsTurning ? Val * Accel : CurrentVal * -1.5f * AutoCorrect;

		// Smoothly interpolate to target yaw speed
		return FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaSeconds, 2.f);
	}

	inline float CalcThrustVelocity(float Val, const float CurrentSpeed, const float Accel, const float MinSpeed, const float MaxSpeed, const float DeltaSeconds)
	{
		float CurrentAcc = (!FMath::IsNearlyEqual(Val, 0.f)) ? (Val * Accel) : (CurrentSpeed < 0.0 ? 0.5f * Accel : -0.5f * Accel);
		// Calculate new speed
		float NewForwardSpeed = CurrentSpeed + (DeltaSeconds * CurrentAcc);
		// Clamp between MinSpeed and MaxSpeed
		return FMath::Clamp(NewForwardSpeed, MinSpeed, MaxSpeed);
	}

	//void LogSyncState(const FZhoenusMovementSyncState& sync)
	//{
	//	FRotator Rotator{ sync.Rotation.Rotator() };
	//	UE_LOG(LogZhoenusPawnSimulation, Log, TEXT("Loc: X=%.2f Y=%.2f Z=%.2f"), sync.Location.X, sync.Location.Y, sync.Location.Z);
	//	UE_LOG(LogZhoenusPawnSimulation, Log, TEXT("Vel: X=%.2f Y=%.2f Z=%.2f"), sync.Velocity.X, sync.Velocity.Y, sync.Velocity.Z);
	//	UE_LOG(LogZhoenusPawnSimulation, Log, TEXT("Rot: P=%.2f Y=%.2f R=%.2f"), Rotator.Pitch, Rotator.Yaw, Rotator.Roll);
	//	UE_LOG(LogZhoenusPawnSimulation, Log, TEXT("Forward: %.2f Pitch: %.2f Yaw: %.2f Roll: %.2f"), sync.CurrentForwardSpeed, sync.CurrentPitchSpeed, sync.CurrentYawSpeed, sync.CurrentRollSpeed);
	//}
}

void FZhoenusMovementSimulation::SimulationTick(const FNetSimTimeStep& TimeStep, const TNetSimInput<ZhoenusMovementStateTypes>& Input, const TNetSimOutput<ZhoenusMovementStateTypes>& Output)
{
	//static int count{ 0 };
	//if (count % 100 == 0)
	//{
	//	UE_LOG(LogZhoenusPawnSimulation, Log, TEXT("Input Sync:"));
	//	LogSyncState(*Input.Sync);
	//	UE_LOG(LogZhoenusPawnSimulation, Log, TEXT("Output Sync:"));
	//	LogSyncState(*Output.Sync);
	//}
	//++count;

	*Output.Sync = *Input.Sync;

	const float DeltaSeconds = (float)TimeStep.StepMS / 1000.f;
	
	// --------------------------------------------------------------
	//	Rotation Update
	//	We do the rotational update inside the movement sim so that things like server side teleport will work.
	//	(We want rotation to be treated the same as location, with respect to how its updated, corrected, etc).
	//	In this simulation, the rotation update isn't allowed to "fail". We don't expect the collision query to be able to fail the rotational update.
	// --------------------------------------------------------------
	FRotator Rot{ Input.Sync->Rotation.Rotator() };
	Output.Sync->AutoCorrectRate = Input.Cmd->MovementInput.X;
	Output.Sync->CurrentPitchSpeed = CalcTurnVelocity(Input.Cmd->RotationInput.X, Input.Sync->CurrentPitchSpeed, Rot.Pitch, Input.Aux->TurnSpeed, Input.Cmd->MovementInput.X, DeltaSeconds);
	Output.Sync->CurrentYawSpeed = CalcYawVelocity(Input.Cmd->RotationInput.Y, Input.Sync->CurrentYawSpeed, Input.Aux->TurnSpeed, DeltaSeconds);
	Output.Sync->CurrentRollSpeed = CalcTurnVelocity(Input.Cmd->RotationInput.Z, Input.Sync->CurrentRollSpeed, Rot.Roll, Input.Aux->RollSpeed, Input.Cmd->MovementInput.X, DeltaSeconds);
	Output.Sync->CurrentForwardSpeed = CalcThrustVelocity(Input.Cmd->RotationInput.W, Input.Sync->CurrentForwardSpeed, Input.Aux->Acceleration, Input.Aux->MinSpeed, Input.Aux->MaxSpeed, DeltaSeconds);

	FRotator DeltaRotation{ 0, 0, 0 };
	DeltaRotation.Pitch = Output.Sync->CurrentPitchSpeed * DeltaSeconds;
	DeltaRotation.Yaw = Output.Sync->CurrentYawSpeed * DeltaSeconds;
	DeltaRotation.Roll = Output.Sync->CurrentRollSpeed * DeltaSeconds;

	const FQuat OutputQuat = DeltaRotation.Quaternion();

	FVector Movement{ Output.Sync->CurrentForwardSpeed * DeltaSeconds, 0.f, 0.f };
	// ===================================================

	// --------------------------------------------------------------
	//	Calculate the final movement delta and move the update component
	// --------------------------------------------------------------
	if (!Movement.IsNearlyZero(1e-6f))
	{
		FHitResult Hit(1.f);
		SafeMoveUpdatedComponent(Movement, OutputQuat, true, Hit, ETeleportType::None);
		if(UpdatedComponent)
		{
			if (UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(UpdatedComponent))
			{
				const float Val{ Output.Sync->AutoCorrectRate };
				if (FMath::IsNearlyEqual(Val, 0.f))
				{
					MeshComponent->SetAngularDamping(0.f);
					MeshComponent->SetLinearDamping(0.f);
				}
				else
				{
					MeshComponent->SetAngularDamping(20.f * Val);
					MeshComponent->SetLinearDamping(20.f * Val);
				}
			}
		}

		if (Hit.IsValidBlockingHit())
		{
			// Try to slide the remaining distance along the surface.
			SlideAlongSurface(Movement, 1.f-Hit.Time, OutputQuat, Hit.Normal, Hit, true);
		}

	}

	// Finalize. This is unfortunate. The component mirrors our internal motion state and since we call into it to update, at this point, it has the real position.
	const FTransform UpdateComponentTransform = GetUpdateComponentTransform();
	Output.Sync->Location = UpdateComponentTransform.GetLocation();
	Output.Sync->Rotation = UpdateComponentTransform.GetRotation();
	Output.Sync->Velocity = Output.Sync->Rotation.RotateVector(Movement);


	// Note that we don't pull the rotation out of the final update transform. Converting back from a quat will lead to a different FRotator than what we are storing
	// here in the simulation layer. This may not be the best choice for all movement simulations, but is ok for this one.
}

void FZhoenusMovementSimulation::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{	

}