// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SaveThemAllGameState.generated.h"

UCLASS(MinimalAPI)
class ASaveThemAllGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ASaveThemAllGameState();

        UPROPERTY(BlueprintReadWrite)
        int32 Saved;

        UPROPERTY(BlueprintReadWrite)
        int32 Total;

};



