// Copyright 2018 loadngo Games, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Controller.h"
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
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float deltaSeconds) override;

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

private:
	APawn* target{ nullptr };
	float currentStateEntered{ 0.f };
	
};
