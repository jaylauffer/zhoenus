// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZhoenusGameMode.h"
#include "ZhoenusSoccerGameMode.generated.h"

UCLASS(MinimalAPI)
class AZhoenusSoccerGameMode : public AZhoenusGameMode
{
	GENERATED_BODY()

public:
	AZhoenusSoccerGameMode();

	void IncrementScore(int32 index, int32 amount);
	UFUNCTION(BlueprintCallable)
	void ClearAfterGoal();
};



