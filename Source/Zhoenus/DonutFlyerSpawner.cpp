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

void ADonutFlyerSpawner::BeginPlay()
{
	Super::BeginPlay();

	UWorld* w{ GetWorld() };
	//FBox box{ALevelBounds::CalculateLevelBounds(w->GetCurrentLevel())};
	FBox box{ SpawnBounds->Bounds.GetBox() };
	FActorSpawnParameters sp{};
	sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	int32 i{ SpawnAmount };

	while (i > 0)
	{
		FVector spawn{ FMath::RandPointInBox(box) };
		FVector start{ spawn };
		FVector end{ start };
		end.Z -= box.GetSize().Z;
		FHitResult hit{};
		if (w->LineTraceSingleByChannel(hit, start, end, ECollisionChannel::ECC_Visibility))
		{
			FRotator rot{ };
			ADonutFlyerPawn* pawn = w->SpawnActor<ADonutFlyerPawn>(spawn, rot, sp);
			if (pawn)
			{
				--i;
			}
		}
	}
}
