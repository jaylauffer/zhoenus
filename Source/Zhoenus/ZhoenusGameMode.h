// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ScoreKeeper.h"
#include "ZhoenusGameMode.generated.h"

UCLASS(MinimalAPI)
class AZhoenusGameMode : public AGameModeBase, public IScoreKeeperInterface
{
	GENERATED_BODY()

public:
	AZhoenusGameMode();
	void Score(AGoal*, APawn*, APawn*) {}
};



