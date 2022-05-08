// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DonutFlyerSpawner.generated.h"

UCLASS(Config=Game)
class ADonutFlyerSpawner : public AActor
{
	GENERATED_BODY()

public:
	ADonutFlyerSpawner();

	// Begin AActor overrides
	virtual void BeginPlay() override;
	// End AActor overrides

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBoxComponent* SpawnBounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpawnAmount;
};
