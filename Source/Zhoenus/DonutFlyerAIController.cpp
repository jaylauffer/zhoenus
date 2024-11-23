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
	static FVector Chasing(ADonutFlyerPawn* ship, const FVector & targetLoc, FVector &lastChase)
	{
		FVector theChaseBefore{ lastChase };
		FVector Start{ ship->GetActorLocation() };
		{
			//if (lastChase != targetLoc)
			{
				// - is forward + is back
				ship->ThrustInput(-0.05f);
			}
		}
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
	//if (GEngine) {
	//	GEngine->AddOnScreenDebugMessage(4, 15.0f, FColor::White, FString::Printf(TEXT("DonutFlyer: Current State: %d"), currentState));
	//}
	switch (currentState)
	{
	case IDLE:
		if (CurrentTimeSeconds - currentStateEntered > 2 * 1 / 60)
		{
			currentState = SEARCHING;
			currentStateEntered = CurrentTimeSeconds;
			//UE_LOG(LOG_TEST, Log, TEXT("Idle switch to searching"));
		}
		break;
	case SEARCHING:
		{
			lastChase.reset();

			//FVector End{ Start.X + 1};
			FCollisionQueryParams CollisionParams;
			//CollisionParams.AddIgnoredActor(target);
			CollisionParams.AddIgnoredActor(pawn);
			TArray<FOverlapResult> overlaps;
			TArray<APawn *> candidates{};
			if (GetWorld()->OverlapMultiByChannel(overlaps, Start, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(SearchDistance), CollisionParams))
			{
				//for now we take the first Zhoenus Pawn we find... 
				for(auto &ov : overlaps)
				{
					if (ov.GetActor()->IsA<ASpaceshipPawn>())
					{
						FHitResult hit{};
						//TODO: make a series of line traces (in case target is partially visible)
						if (GetWorld()->LineTraceSingleByChannel(hit, Start, ov.GetActor()->GetActorLocation(), ECC_WorldDynamic, CollisionParams))
						{
							if (auto ZhoenusPawn = Cast<ASpaceshipPawn>(hit.GetActor()))
							{
								auto &ps = AggroMap.FindOrAdd(ZhoenusPawn);
								ps.SeenAggro += 3.f + ((SearchDistance - hit.Distance) / SearchDistance);
								//UE_LOG(LOG_TEST, Log, TEXT("Seen player: %g - %g - %g, %s"), ps.BumpAggro, ps.ShotAggro, ps.SeenAggro, *pc.GetName());
								candidates.Push(ZhoenusPawn);
							}
						}
					}
				}
				if (GetTargetPlayer(candidates))
				{
					currentState = CHASING;
					currentStateEntered = CurrentTimeSeconds;
					//UE_LOG(LOG_TEST, Log, TEXT("Searching switch to chasing"));
				}
				//TODO: add ROAMING state and after a certain period of failed searching being to roam....
			}
		}
		break;
	case CHASING:
		if (APawn *target = GetTargetPlayer())
		{
			FVector targetLoc{ target->GetActorLocation() };
			pawn->DisengageAutoCorrect(0.0f);

			FVector ForwardVector{ pawn->GetActorForwardVector() };
			FVector End{ ((ForwardVector * 450.f) + Start) };
			FCollisionQueryParams CollisionParams;
			//CollisionParams.AddIgnoredActor(target);
			CollisionParams.AddIgnoredActor(pawn);
			FHitResult OutHit;
			if (pawn->GetDistanceTo(target) < 300.f)
			{
				pawn->ThrustInput(0.0f);
				currentState = HOVERING;
				currentStateEntered = CurrentTimeSeconds;
				//UE_LOG(LOG_TEST, Log, TEXT("Chasing switch to hovering 1"));
			}
			else if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldDynamic, CollisionParams))
			{
				pawn->ThrustInput(0.0f);
				if (Cast<APawn>(OutHit.GetActor()) == target)
				{
					currentState = HOVERING;
					currentStateEntered = CurrentTimeSeconds;
					//UE_LOG(LOG_TEST, Log, TEXT("Chasing switch to hovering 2"));
				}
				else if (pawn->GetVelocity().Size() < 1.0f && CurrentTimeSeconds - currentStateEntered > 2.f)
				{
					currentState = STUCK;
					currentStateEntered = CurrentTimeSeconds;
					AActor* a{ OutHit.GetActor() };
					//UE_LOG(LOG_TEST, Log, TEXT("Chasing switch to stuck: %s"), a ? *a->GetName() : TEXT("nullptr"));
				}
				else if (Start == PreviousLocation && CurrentTimeSeconds - currentStateEntered > 3.f)
				{
					currentState = STUCK;
					currentStateEntered = CurrentTimeSeconds;
					//UE_LOG(LOG_TEST, Log, TEXT("Chasing switch to stuck: no movement"));
				}
			}
			else
			{
				//if (lastChase != targetLoc)
				{
					float DistanceToTarget = FVector::Dist(Start, targetLoc);
					float ScaledThrust = FMath::Clamp(DistanceToTarget / 1000.0f, 0.f, 0.5f);
					// - is forward + is back
					pawn->ThrustInput(-ScaledThrust);
				}
			}
			pawn->TargetRot = (targetLoc - Start).Rotation();
			lastChase = targetLoc;
		}
		else
		{
			currentState = SEARCHING;
			currentStateEntered = CurrentTimeSeconds;
			//UE_LOG(LOG_TEST, Log, TEXT("Chasing switch to searching"));
		}
		break;
	case HOVERING:
		if (APawn* target = GetTargetPlayer())
		{

			FVector ForwardVector{ pawn->GetActorForwardVector() };
			FVector End{ Start + (ForwardVector * 200.f) };
			FCollisionQueryParams CollisionParams;
			//CollisionParams.AddIgnoredActor(target);
			CollisionParams.AddIgnoredActor(pawn);
			FHitResult OutHit;
			GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldDynamic, CollisionParams);
			if (Cast<APawn>(OutHit.GetActor()) != target)
			{
				currentState = CHASING;
				currentStateEntered = CurrentTimeSeconds;
				AActor* a{ OutHit.GetActor() };
				//UE_LOG(LOG_TEST, Log, TEXT("Hovering switch to chasing: %s"), a ? *a->GetName() : TEXT("nullptr"));
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
			//UE_LOG(LOG_TEST, Log, TEXT("Hovering switch to searching"));
		}
		break;
	case LOCKED:
		if (!lastChase)
		{
			lastChase = FVector{};
		}
		Chasing(GetPawn<ADonutFlyerPawn>(), LockedLocation, lastChase.value());

		if (CurrentTimeSeconds - currentStateEntered > 2.f && Start == PreviousLocation) {
			// Velocity-based stuck detection
			FVector Velocity = GetPawn()->GetVelocity();
			float Speed = Velocity.Size();
			if (Speed < 1.f) // If speed is too low, likely stuck
			{
				currentState = STUCK;
				currentStateEntered = CurrentTimeSeconds;
				UE_LOG(LOG_TEST, Warning, TEXT("LOCKED mode detected low velocity, switching to STUCK."));
			}
		}
		break;
	case STUCK:
		//the donutflyer can not reach the target.. because of some obstacle..
		//this may be because a player is under the "floor" for example
		//in this case we want the donutflyer to attempt to find a path to the player..
		//but for now for simplicity we may just accelerate aggro depletion
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
			//CollisionParams.AddIgnoredActor(target);
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
