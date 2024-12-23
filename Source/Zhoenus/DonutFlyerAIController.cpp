// Copyright 2018 loadngo Games, All rights reserved

#include "DonutFlyerAIController.h"
#include "DonutFlyerPawn.h"
#include "SpaceshipPawn.h"
#include "Containers/Map.h"
#include "GameFramework/PlayerController.h"
#include "ZhoenusPlayerState.h"
#include "Runtime/Engine/Classes/Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LOG_TEST, Log, All);

namespace
{
	static FVector Chasing(ADonutFlyerPawn* ship, const FVector& targetLoc, FVector& lastChase)
	{
		FVector theChaseBefore{ lastChase };
		FVector Start{ ship->GetActorLocation() };
		float DistanceToTarget = FVector::Dist(Start, targetLoc);

		// Reduce thrust input as the flyer gets closer to the target
		float ScaledThrust = FMath::Clamp(DistanceToTarget / 1000.0f, 0.f, 0.5f);
		ship->ThrustInput(-ScaledThrust);

		ship->TargetRot = (targetLoc - Start).Rotation();
		lastChase = targetLoc;
		return theChaseBefore;
	}
}

ADonutFlyerAIController::ADonutFlyerAIController()
{
	//first edition LockedTarget is null
	LockedTarget = nullptr;
	bIsPlayerController = false;
}

float ADonutFlyerAIController::playerTargetScore(APawn* pawn)
{
	auto ps = AggroMap.Find(pawn);
	if (ps)
	{
		return CalcAggro(*ps);
	}
	return 0.f;
}

APawn* ADonutFlyerAIController::GetTargetPlayer()
{
	int playerCount{ 0 };
	APawn* ret{ nullptr };
	float max_score{ 0.0 };
	TArray<AActor*> cleanup;
	for (auto &elem : AggroMap) 
	{
		if (IsValid(elem.Key))
		{
			float s = CalcAggro(elem.Value);
			if (s > max_score && !elem.Key->IsPendingKillPending())
			{
				ret = elem.Key;
				max_score = s;
			}
			//UE_LOG(LOG_TEST, Warning, TEXT("Player #: %d, shoot: %d, hit: %d, score: %f, max_score: %f"), playerCount+1, ps->Shoot, ps->Hit, s, max_score);
			++playerCount;
		}
		else
		{
			cleanup.Add(elem.Key);
		}
	}
	//wipe out any invalid pawns
	if (cleanup.Num())
	{
		for (auto& p : cleanup)
		{
			if (ASpaceshipPawn* hope = Cast<ASpaceshipPawn>(p))
			{
				ADonutFlyerPawn* dnt{ GetPawn<ADonutFlyerPawn>() };
				hope->Followers.Remove(dnt);
				AggroMap.Remove(hope); //we're not really going to remove hope though d-;
			}

		}
	}
	// update target player's status.
	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; it++)
	{
		APlayerController* pc = it->Get();
		if(auto pawn = Cast<ASpaceshipPawn>(pc->GetPawn()))
		{
			pawn->IsDonutTarget = pawn == ret;
			pawn->Followers.Add(this->GetPawn<ADonutFlyerPawn>());
		}
	}
	
	return ret;
}

APawn* ADonutFlyerAIController::GetTargetPlayer(TArray<APawn *> players)
{
	APawn* ret{ nullptr };
	float max_score{ 0.0 };
	for (APawn* pawn : players) {
		float s = this->playerTargetScore(pawn);
		if (s > max_score) {
			ret = pawn;
			max_score = s;
		}
//		UE_LOG(LOG_TEST, Warning, TEXT("Player #: %d, shoot: %d, hit: %d, score: %f, max_score: %f"), playerCount+1, ps->Shoot, ps->Hit, s, max_score);
	}
	return ret;
}

static void ReduceAggro(float& Aggro, float PercentDecrease, float DeltaSeconds)
{
	Aggro = FMath::Max(0.f, Aggro - (Aggro * PercentDecrease * DeltaSeconds));
}

void ADonutFlyerAIController::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);
	float CurrentTimeSeconds = GetWorld()->GetTimeSeconds();
	ADonutFlyerPawn* pawn{ Cast<ADonutFlyerPawn>(GetPawn()) };
	FVector Start{ pawn->GetActorLocation() };
	UpdateQuatHistory(pawn);
	switch (currentState)
	{
	case IDLE:
		if (CurrentTimeSeconds - currentStateEntered > 2 * 1 / 60)
		{
			currentState = SEARCHING;
			currentStateEntered = CurrentTimeSeconds;
		}
		break;
	case SEARCHING:
	{
		lastChase.reset();
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(pawn);
		TArray<FOverlapResult> overlaps;
		TArray<APawn*> candidates{};
		if (GetWorld()->OverlapMultiByChannel(overlaps, Start, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(SearchDistance), CollisionParams))
		{
			for (auto& ov : overlaps)
			{
				if (ov.GetActor()->IsA<ASpaceshipPawn>())
				{
					FHitResult hit{};
					if (GetWorld()->LineTraceSingleByChannel(hit, Start, ov.GetActor()->GetActorLocation(), ECC_WorldDynamic, CollisionParams))
					{
						if (auto ZhoenusPawn = Cast<ASpaceshipPawn>(hit.GetActor()))
						{
							auto& ps = AggroMap.FindOrAdd(ZhoenusPawn);
							ps.SeenAggro += 3.f + ((SearchDistance - hit.Distance) / SearchDistance);
							candidates.Push(ZhoenusPawn);
						}
					}
				}
			}
			if (GetTargetPlayer(candidates))
			{
				currentState = CHASING;
				currentStateEntered = CurrentTimeSeconds;
			}
		}
	}
	break;
	case CHASING:
	{
		if (APawn* target = GetTargetPlayer())
		{
			// Check if target is a spaceship (i.e. a player's ship)
			if (ASpaceshipPawn* spaceshipTarget = Cast<ASpaceshipPawn>(target))
			{
				// ================
				//  GENTLE APPROACH
				// ================
				pawn->DisengageAutoCorrect(0.0f);

				float distance = pawn->GetDistanceTo(spaceshipTarget);
				if (distance < 300.f)
				{
					// If close to a player's ship, hover
					pawn->ThrustInput(0.0f);
					currentState = HOVERING;
					currentStateEntered = CurrentTimeSeconds;
				}
				else
				{
					// Gentle forward thrust (negative is forward in your code)
					float DistanceFactor = distance / 2000.0f;
					float MaxForwardThrust = 0.25f;
					float ThrustValue = -FMath::Clamp(DistanceFactor, 0.f, MaxForwardThrust);
					pawn->ThrustInput(ThrustValue);

					// Rotate toward the target
					pawn->TargetRot = (spaceshipTarget->GetActorLocation() - Start).Rotation();
				}
			}
			else
			{
				// ==================
				//  NON-PLAYER TARGET
				// ==================
				// e.g. Gate of Oblivion (AGoal)
				// Move at full speed, do not hover
				float FullSpeed = -1.0f;  // negative is forward
				pawn->ThrustInput(FullSpeed);

				pawn->TargetRot = (target->GetActorLocation() - Start).Rotation();
			}

			lastChase = target->GetActorLocation();
		}
		else
		{
			currentState = SEARCHING;
			currentStateEntered = CurrentTimeSeconds;
		}
	}
	break;

	case HOVERING:
		if (APawn* target = GetTargetPlayer())
		{
			FVector ForwardVector{ pawn->GetActorForwardVector() };
			FVector End{ Start + (ForwardVector * 200.f) };
			FCollisionQueryParams CollisionParams;
			CollisionParams.AddIgnoredActor(pawn);
			FHitResult OutHit;
			GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldDynamic, CollisionParams);
			if (Cast<APawn>(OutHit.GetActor()) != target)
			{
				currentState = CHASING;
				currentStateEntered = CurrentTimeSeconds;
			}
			else if (pawn->GetDistanceTo(target) < 300.f)
			{
				pawn->DisengageAutoCorrect(10.0f);
			}
			else
			{
				pawn->DisengageAutoCorrect(0.0f);
			}
			FVector targetLoc{ target->GetActorLocation() };
			pawn->TargetRot = (targetLoc - Start).Rotation();
		}
		else
		{
			currentState = SEARCHING;
			currentStateEntered = CurrentTimeSeconds;
		}
		break;
	case LOCKED:
	{
		if (!lastChase)
		{
			lastChase = FVector{};
		}

		if (LockedTarget) // We have a locked target
		{
			{
				// ==============
				//  NON-PLAYER
				// ==============
				// e.g., Gate of Oblivion
				// Use full speed for locked non-player targets
				pawn->ThrustInput(-1.0f);
				pawn->TargetRot = (LockedLocation - Start).Rotation();
			}

			if (IsCirclingByQuaternions())
			{
				// Break circling by stopping thrust momentarily, subtle lateral input
				pawn->ThrustInput(0.0f);
				pawn->MoveRightInput(FMath::RandRange(-0.2f, 0.2f));
			}

			// If stuck
			if (CurrentTimeSeconds - currentStateEntered > 2.f && Start == PreviousLocation)
			{
				currentState = STUCK;
				currentStateEntered = CurrentTimeSeconds;
				UE_LOG(LOG_TEST, Warning, TEXT("LOCKED mode detected low velocity, switching to STUCK."));
			}
		}
		else
		{
			// If no LockedTarget anymore, revert to searching or some default
			currentState = SEARCHING;
			currentStateEntered = CurrentTimeSeconds;
		}

		lastChase = LockedLocation;
	}
	break;

	case STUCK:
		if (LockedTarget)
		{
			if (CurrentTimeSeconds - currentStateEntered > 8.f)
			{
				currentState = LOCKED;
				currentStateEntered = CurrentTimeSeconds;
			}
		}
		else if (APawn* target = GetTargetPlayer())
		{
			FVector TargetVector{ target->GetActorLocation() - Start };
			FVector End{ Start + (TargetVector.Rotation().Quaternion().Vector() * SearchDistance) };
			FCollisionQueryParams CollisionParams;
			CollisionParams.AddIgnoredActor(pawn);
			FHitResult OutHit;
			GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldDynamic, CollisionParams);
			if (Cast<APawn>(OutHit.GetActor()) != target)
			{
				if (APlayerController* pc = target->GetController<APlayerController>())
				{
					if (auto ps = pc->GetPlayerState<AZhoenusPlayerState>())
					{
						ReduceAggro(ps->BumpAggro, 0.0542f, deltaSeconds);
						ReduceAggro(ps->ShotAggro, 0.0542f, deltaSeconds);
						ReduceAggro(ps->SeenAggro, 0.0542f, deltaSeconds);
					}
				}
			}
			else
			{
				currentState = CHASING;
				currentStateEntered = CurrentTimeSeconds;
				UE_LOG(LOG_TEST, Log, TEXT("Stuck switch to chasing"));
			}
		}
		else
		{
			currentState = SEARCHING;
			currentStateEntered = CurrentTimeSeconds;
			UE_LOG(LOG_TEST, Log, TEXT("Stuck switch to searching"));
		}
	}
	DecreaseAggro(deltaSeconds);
	PreviousLocation = Start;
}

void ADonutFlyerAIController::UpdateQuatHistory(APawn *Donut)
{

	// Get current orientation as a quaternion
	FQuat CurrentQuat = Donut->GetActorRotation().Quaternion();

	// Add to our history
	QuatHistory.Add(CurrentQuat);

	// Keep array size under control
	if (QuatHistory.Num() > MaxQuatSamples)
	{
		QuatHistory.RemoveAt(0);
	}
}

bool ADonutFlyerAIController::IsCirclingByQuaternions() const
{
	// Need at least 2 quaternions to compare a rotation
	if (QuatHistory.Num() < 2)
	{
		return false;
	}

	// We'll accumulate rotation in degrees
	float AccumulatedRotationDeg = 0.0f;

	// Iterate over consecutive pairs
	for (int32 i = 1; i < QuatHistory.Num(); ++i)
	{
		const FQuat& PrevQuat = QuatHistory[i - 1];
		const FQuat& CurrQuat = QuatHistory[i];

		// Relative rotation: Q_diff = Curr * Inverse(Prev)
		FQuat DiffQuat = CurrQuat * PrevQuat.Inverse();

		// Normalize the difference in case of floating point drift
		DiffQuat.Normalize();

		// Convert DiffQuat to an axis and angle
		// angle is in radians, axis is a unit vector
		FVector Axis;
		float AngleRad;
		DiffQuat.ToAxisAndAngle(Axis, AngleRad);

		// Convert to degrees
		float AngleDeg = FMath::RadiansToDegrees(AngleRad);

		// For total rotation in 3D, just accumulate the absolute angle
		// (This does NOT distinguish direction or axis—any rotation counts.)
		AccumulatedRotationDeg += AngleDeg;
	}

	// If the total rotation across these samples exceeds our threshold
	// (e.g., 360 deg = a full revolution), consider it circling
	return AccumulatedRotationDeg >= DegreesThreshold;
}

void ADonutFlyerAIController::DecreaseAggro(float DeltaSeconds)
{
	for (auto &aggro : AggroMap)
	{
		if (CalcAggro(aggro.Value) > 0.f)
		{
			ReduceAggro(aggro.Value.BumpAggro, 0.0231f, DeltaSeconds);
			ReduceAggro(aggro.Value.ShotAggro, 0.0456f, DeltaSeconds);
			ReduceAggro(aggro.Value.SeenAggro, 0.00931f, DeltaSeconds);
		}
	}
}


APawn* ADonutFlyerAIController::LockTarget(AActor* goal, const FVector &Location)
{
	if (!LockedTarget) //Zhoenus first edition, Zhoenus I locked target is set once and will not be changed, 
		//this can be explored in post-quantum era to enable more bountiful gameplay experiences
	{
		LockedTarget = goal;
		LockedLocation = Location;
		currentState = LOCKED;
		currentStateEntered = GetWorld()->GetTimeSeconds();
		//TODO: assigning locked target should emit some event with the target.. (sparks of joy for example)
	}
	return Cast<APawn>(goal); //we give back the goal .. and it may have altered..
}

void ADonutFlyerAIController::OnUnPossess()
{
	Super::OnUnPossess();
}

void ADonutFlyerAIController::BeginPlay()
{
	Super::BeginPlay(); // Call the parent class's BeginPlay function
	LockedTarget = nullptr; //always begin with no locked target
	PreviousLocation = FVector{};
	// Your initialization code here
}
