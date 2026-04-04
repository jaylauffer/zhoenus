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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 SpawnAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float MinSpawnHeightAboveGround{ 100.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 MaxSpawnAttemptsPerFlyer{ 20 };

	UPROPERTY(BlueprintReadOnly, Category = "Spawn")
	int32 LastSpawnedCount{ 0 };

private:
	bool TryFindSpawnLocation(class UWorld* World, const FBox& SpawnBox, FVector& OutSpawnLocation) const;
};
