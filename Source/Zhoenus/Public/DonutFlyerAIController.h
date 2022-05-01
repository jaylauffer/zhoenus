// Copyright 2018 loadngo Games, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameEngine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DonutFlyerAIController.generated.h"


/**
 * 
 */
UCLASS()
class ZHOENUS_API ADonutFlyerAIController : public AController
{
	GENERATED_BODY()
	
public:	
	ADonutFlyerAIController();

	virtual void Tick(float deltaSeconds) override;
	APawn * GetTargetPlayer();
	APawn * GetTargetPlayer(TArray<APawn *> players);

	enum DonutState
	{
		IDLE,
		SEARCHING,
		CHASING,
		HOVERING,
		TARGETING,
		STUCK,
	};

	DonutState currentState{ IDLE };

	UPROPERTY(BlueprintReadWrite)
	float SearchDistance{ 500.f };

private:
	float currentStateEntered{ 0.f };
	float playerTargetScore(APawn* pawn);
	void DecreaseAggro(float deltaSeconds);

	struct PawnAggro
	{
		float BumpAggro;
		float ShotAggro;
		float SeenAggro;
	};

	FORCEINLINE float CalcAggro(const PawnAggro& pa) { return pa.BumpAggro + pa.ShotAggro + pa.SeenAggro; }

	TMap<APawn*, PawnAggro> AggroMap;
};
