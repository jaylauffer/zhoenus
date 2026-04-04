#pragma once

#include "CoreMinimal.h"
#include "ScoreKeeper.generated.h"

UINTERFACE(meta = (CannotImplementInterfaceInBlueprint))
class UScoreKeeperInterface : public UInterface
{
	GENERATED_BODY()
};

class AGoal;

class IScoreKeeperInterface
{
	GENERATED_BODY()

public:
	virtual void Score(AGoal *goal, APawn *player, APawn *ball) = 0;
};