//Copyright Jay Lauffer 2026

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



