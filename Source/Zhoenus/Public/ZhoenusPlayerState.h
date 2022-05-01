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
	void OnDonutHitFromMe(float impactMagnitude);

	UFUNCTION()
	void OnDonutShootedFromMe();

	/** Aggro score */
	UPROPERTY(Category = RecordsOfDonut, EditAnywhere)
	float BumpAggro;

	UPROPERTY(Category = RecordsOfDonut, EditAnywhere)
	float ShotAggro;

	UPROPERTY(Category = RecordsOfDonut, EditAnywhere)
	float SeenAggro;


	/** Clear shoot and hit after goal**/
	void ClearPlayerState();
};
