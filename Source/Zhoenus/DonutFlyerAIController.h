// Copyright 2018 loadngo Games, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "DonutAggroTuning.h"
#include "GameFramework/Controller.h"
#include "Containers/Map.h"
#include <optional>

#include "DonutFlyerAIController.generated.h"


class ADonutFlyerPawn;
class ASpaceshipPawn;
class APlayerController;
class AZhoenusPlayerState;

enum class EDonutAggroEventType : uint8
{
	SightPulse,
	ProjectileNearMiss,
	ProjectileHit,
	CollisionBump,
	ScriptedThreat,
};

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
	void ApplyAggroEvent(APawn* AggroSource, EDonutAggroEventType EventType, float AggroAmount, bool bWakeDonut = true);

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
	void DecreaseThreat(float deltaSeconds);
	FVector GetLockedTargetCenter(FVector* TargetExtent = nullptr) const;
	FVector GetLockedApproachPoint(const FVector& Start, FVector* TargetExtent = nullptr) const;
	void ResetLockedTracking(float CurrentTimeSeconds);
	std::optional<FVector> lastChase;

	struct FPlayerThreatState
	{
		float CollisionThreat{ 0.f };
		float ProjectileThreat{ 0.f };
		float SightThreat{ 0.f };
	};

	FORCEINLINE float CalcThreatScore(const FPlayerThreatState& ThreatState) const
	{
		return ThreatState.CollisionThreat + ThreatState.ProjectileThreat + ThreatState.SightThreat;
	}

	TMap<APawn*, FPlayerThreatState> ThreatMap;
	FDonutAggroTuning ThreatTuning;

	//this is the ultimate objective, for example, the gate of oblivion, the flyer will lock the gate of oblivion as their target
	//once a player successfully triggers the lock (by flying through the gate)
	//the AI will then return LockedTarget instead of reevaluating the score.. the mechanics of this can be revisited
	//and expanded to enable more gameplay options, for the first edition, lock target always stays true
	AActor* LockedTarget;
	FVector LockedLocation;
	FVector PreviousLocation;
	float PreviousLockedDistance{ 0.f };
	float LastLockedProgressTime{ 0.f };
	bool bHasLockedDistanceSample{ false };

public:
	// Number of orientation samples we keep.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circling|Quaternion")
	int32 MaxQuatSamples = 60;

	// Threshold for how many degrees of rotation we consider "one circle."
	// If we want a full circle = 360 degrees. 
	// If we only need 75% of a rotation, set something like 270.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circling|Quaternion")
	float DegreesThreshold = 360.0f;

protected:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// Store orientation quaternions from recent frames
	TArray<FQuat> QuatHistory;

	// Update array with current orientation each tick
	void UpdateQuatHistory(APawn *Donut);

	// Check if circling based on heading changes.
	bool IsCirclingByQuaternions() const;

};
