// Copyright 2018 loadngo Games, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ZhoenusPlayerState.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogZhoenusPlayerState, Log, All);
/**
 * 
 */
UCLASS(Config = Game)
class ZHOENUS_API AZhoenusPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AZhoenusPlayerState();

	UFUNCTION()
	void RecordDonutCollision(float impactMagnitude);

	UFUNCTION()
	void RecordDonutShot();

	/** Donut interaction telemetry */
	UPROPERTY(Category = DonutTelemetry, EditAnywhere)
	float DonutCollisionScore;

	UPROPERTY(Category = DonutTelemetry, EditAnywhere)
	float DonutShotScore;

	UPROPERTY(Category = DonutTelemetry, EditAnywhere)
	float DonutSightScore;


	/** Reset donut telemetry after goal **/
	void ResetDonutEngagementStats();
};
