// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "DonutFlyerSpawner.h"
#include "DonutFlyerPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "Components/BoxComponent.h"

ADonutFlyerSpawner::ADonutFlyerSpawner()
{
	SpawnBounds = CreateDefaultSubobject<UBoxComponent>("SpawnBounds");
}

bool ADonutFlyerSpawner::TryFindSpawnLocation(UWorld* World, const FBox& SpawnBox, FVector& OutSpawnLocation) const
{
	if (World == nullptr)
	{
		return false;
	}

	const float SpawnX = FMath::FRandRange(SpawnBox.Min.X, SpawnBox.Max.X);
	const float SpawnY = FMath::FRandRange(SpawnBox.Min.Y, SpawnBox.Max.Y);
	const float SpawnZ = FMath::FRandRange(SpawnBox.Min.Z, SpawnBox.Max.Z);
	const FVector CandidateLocation{ SpawnX, SpawnY, SpawnZ };

	const FVector TraceStart{ SpawnX, SpawnY, SpawnBox.Max.Z };
	const FVector TraceEnd{ SpawnX, SpawnY, SpawnBox.Min.Z - 1000.f };
	FHitResult GroundHit;
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(DonutFlyerSpawnerGroundTrace), false, this);
	if (!World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, TraceParams))
	{
		return false;
	}

	if (CandidateLocation.Z <= GroundHit.Location.Z + MinSpawnHeightAboveGround)
	{
		return false;
	}

	OutSpawnLocation = CandidateLocation;
	return true;
}

void ADonutFlyerSpawner::BeginPlay()
{
	Super::BeginPlay();

	UWorld* w{ GetWorld() };
	//FBox box{ALevelBounds::CalculateLevelBounds(w->GetCurrentLevel())};
	FBox box{ SpawnBounds->Bounds.GetBox() };
	FActorSpawnParameters sp{};
	sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	const int32 MaxAttempts = FMath::Max(SpawnAmount * MaxSpawnAttemptsPerFlyer, SpawnAmount);
	int32 AttemptCount{ 0 };
	LastSpawnedCount = 0;

	while (LastSpawnedCount < SpawnAmount && AttemptCount < MaxAttempts)
	{
		++AttemptCount;

		FVector SpawnLocation;
		if (TryFindSpawnLocation(w, box, SpawnLocation))
		{
			FRotator rot{ };
			ADonutFlyerPawn* pawn = w->SpawnActor<ADonutFlyerPawn>(SpawnLocation, rot, sp);
			if (pawn)
			{
				++LastSpawnedCount;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("DonutFlyerSpawner '%s' requested %d flyers and spawned %d after %d attempts."),
		*GetName(), SpawnAmount, LastSpawnedCount, AttemptCount);

	if (LastSpawnedCount < SpawnAmount)
	{
		UE_LOG(LogTemp, Warning, TEXT("DonutFlyerSpawner '%s' could not place all flyers above the landscape. Missing %d."),
			*GetName(), SpawnAmount - LastSpawnedCount);
	}
}
