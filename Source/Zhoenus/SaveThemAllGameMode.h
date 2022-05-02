// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZhoenusGameMode.h"
#include "SaveThemAllGameMode.generated.h"

UCLASS(MinimalAPI)
class ASaveThemAllGameMode : public AZhoenusGameMode
{
	GENERATED_BODY()

public:
	ASaveThemAllGameMode();

	virtual void Score(AGoal *goal, APawn *player, APawn *ball) override;
};



