// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ZhoenusSoccerGameState.generated.h"

UCLASS(MinimalAPI)
class AZhoenusSoccerGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AZhoenusSoccerGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, BlueprintReadWrite)
	TArray<int32> Score;
};



