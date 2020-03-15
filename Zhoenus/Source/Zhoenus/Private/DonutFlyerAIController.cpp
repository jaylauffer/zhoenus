// Copyright 2018 loadngo Games, All rights reserved

#include "DonutFlyerAIController.h"
#include "DonutFlyerPawn.h"
#include "ZhoenusPawn.h"
#include "Map.h"
#include "GameFramework/PlayerController.h"
#include "Runtime/Engine/Classes/Engine/World.h"

static APawn * GetPlayerPawn(UWorld *world)
{
	FConstPlayerControllerIterator iter{ world->GetPlayerControllerIterator() };
	APlayerController *pc{ iter ? iter->Get() : nullptr };
	return pc ? pc->GetPawn() : nullptr;
}

ADonutFlyerAIController::ADonutFlyerAIController()
{
	bIsPlayerController = false;
}

void ADonutFlyerAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	target = GetPlayerPawn(GetWorld());
}

void ADonutFlyerAIController::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);
	ADonutFlyerPawn* pawn{ Cast<ADonutFlyerPawn>(GetPawn()) };
	switch (currentState)
	{
	case IDLE:
		if (deltaSeconds - currentStateEntered > 2 * 1 / 60)
		{
			currentState = SEARCHING;
			currentStateEntered = deltaSeconds;
		}
		break;
	case SEARCHING:
		{
			FVector Start{ pawn->GetActorLocation() };
			//FVector End{ Start.X + 1};
			FCollisionQueryParams CollisionParams;
			//CollisionParams.AddIgnoredActor(target);
			CollisionParams.AddIgnoredActor(pawn);
			TArray<FOverlapResult> overlaps;
			target = nullptr;
			if (GetWorld()->OverlapMultiByChannel(overlaps, Start, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(1350.f), CollisionParams))
			{
				//for now we take the first Zhoenus Pawn we find... 
				for(auto &ov : overlaps)
				{
					if (ov.GetActor()->IsA<AZhoenusPawn>())
					{
						FHitResult hit{};
						//TODO: make a series of line traces (in case target is partially visible)
						if (GetWorld()->LineTraceSingleByChannel(hit, Start, ov.GetActor()->GetActorLocation(), ECC_WorldDynamic, CollisionParams))
						{
							if (hit.GetActor()->IsA<AZhoenusPawn>())
							{
								target = Cast<APawn>(hit.GetActor());
								currentState = CHASING;
								currentStateEntered = deltaSeconds;
								break;
							}
						}
					}
				}
				//TODO: add ROAMING state and after a certain period of failed searching being to roam....
			}
		}
		break;
	case CHASING:
		if (target)
		{
			FVector Start{ pawn->GetActorLocation() };
			FVector ForwardVector{ pawn->GetActorForwardVector() };
			FVector End{ ((ForwardVector * 150.f) + Start) };
			FCollisionQueryParams CollisionParams;
			//CollisionParams.AddIgnoredActor(target);
			CollisionParams.AddIgnoredActor(pawn);
			FHitResult OutHit;
			if (pawn->GetDistanceTo(target) < 200.f)
			{
				pawn->ThrustInput(0.f);
				currentState = HOVERING;
				currentStateEntered = deltaSeconds;
			}
			else if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldDynamic, CollisionParams))
			{
				pawn->ThrustInput(0.f);
				if (Cast<APawn>(OutHit.GetActor()) == target)
				{
					currentState = HOVERING;
					currentStateEntered = deltaSeconds;
				}
				else if (deltaSeconds - currentStateEntered > 2.f)
				{
					currentState = STUCK;
					currentStateEntered = deltaSeconds;
				}
			}
			else
			{
				// - is forward + is back
				pawn->ThrustInput(-0.05f);
			}
			FVector targetLoc{ target->GetActorLocation() };
			pawn->TargetRot = (targetLoc - Start).Rotation();
		}
		else
		{
			currentState = SEARCHING;
			currentStateEntered = deltaSeconds;
		}
		break;
	case HOVERING:
		if (target)
		{
			FVector Start{ pawn->GetActorLocation() };
			FVector TargetVector{ target->GetActorLocation() - Start };
			FVector End{ Start + (TargetVector.Rotation().Quaternion().Vector() * 200.f) };
			FCollisionQueryParams CollisionParams;
			//CollisionParams.AddIgnoredActor(target);
			CollisionParams.AddIgnoredActor(pawn);
			FHitResult OutHit;
			GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldDynamic, CollisionParams);
			if (Cast<APawn>(OutHit.GetActor()) != target)
			{
				currentState = CHASING;
				currentStateEntered = deltaSeconds;
			}
			FVector targetLoc{ target->GetActorLocation() };
			pawn->TargetRot = (targetLoc - Start).Rotation();
		}
		else
		{
			currentState = SEARCHING;
			currentStateEntered = deltaSeconds;
		}
	//case STUCK:

	}
}