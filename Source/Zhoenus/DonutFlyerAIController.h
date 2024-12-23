// Copyright 2018 loadngo Games, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Controller.h"
#include "Containers/Map.h"
#include <optional>

#include "DonutFlyerAIController.generated.h"


class ADonutFlyerPawn;
class ASpaceshipPawn;
class APlayerController;
class AZhoenusPlayerState;

/**
 *
 */
UCLASS()
class ZHOENUS_API ADonutFlyerAIController : public AController
{
	GENERATED_BODY()

public:
	ADonutFlyerAIController();

	//virtual void Tick(float deltaSeconds) override;
	APawn* GetTargetPlayer();
	APawn* GetTargetPlayer(TArray<APawn*> players);
	APawn* LockTarget(AActor* goal, const FVector& Location);

	void OnUnPossess() final;

	enum DonutState
	{
		IDLE,
		SEARCHING,
		CHASING,
		HOVERING,
		TARGETING,
		LOCKED, //this is the breakthrough, enabling locked state allows us to elevate to the next level
		STUCK,
	};

	DonutState currentState{ IDLE };

	UPROPERTY(BlueprintReadWrite)
	float SearchDistance{ 500.f };

protected:
	virtual void BeginPlay() override final;

private:
	float currentStateEntered{ 0.f };
	float playerTargetScore(APawn* pawn);
	void DecreaseAggro(float deltaSeconds);
	std::optional<FVector> lastChase;

	struct PawnAggro
	{
		float BumpAggro;
		float ShotAggro;
		float SeenAggro;
	};

	FORCEINLINE float CalcAggro(const PawnAggro& pa) { return pa.BumpAggro + pa.ShotAggro + pa.SeenAggro; }

	TMap<APawn*, PawnAggro> AggroMap;

	//this is the ultimate objective, for example, the gate of oblivion, the flyer will lock the gate of oblivion as their target
	//once a player successfully triggers the lock (by flying through the gate)
	//the AI will then return LockedTarget instead of reevaluating the score.. the mechanics of this can be revisited
	//and expanded to enable more gameplay options, for the first edition, lock target always stays true
	AActor* LockedTarget;
	FVector LockedLocation;
	FVector PreviousLocation;

public:
	// How many heading samples we keep in memory to compute turn rate.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circling")
	int32 MaxYawSamples = 60;

	// Required average degrees per second to consider the flyer "circling."
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circling")
	float CirclingTurnRateThresholdDegPerSec = 90.0f;

protected:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// Stores yaw angles in degrees from the last N frames.
	TArray<float> YawHistory;

	// Stores timestamps for each yaw sample.
	TArray<float> TimeHistory;

	// Check if circling based on heading changes.
	bool IsCircling() const;

	// Helper to keep track of heading over time.
	void UpdateHeadingHistory(float DeltaTime);
};
