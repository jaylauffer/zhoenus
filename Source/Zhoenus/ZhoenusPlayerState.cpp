// Copyright 2018 loadngo Games, All rights reserved


#include "ZhoenusPlayerState.h"

DEFINE_LOG_CATEGORY(LogZhoenusPlayerState)

AZhoenusPlayerState::AZhoenusPlayerState()
{
	DonutCollisionScore = 0.f;
	DonutShotScore = 0.f;
	DonutSightScore = 0.f;
}

void AZhoenusPlayerState::RecordDonutCollision(float impactMagnitude)
{
	float impactBonus{ 5.f * impactMagnitude / 750.f };
	DonutCollisionScore += impactBonus;
	//UE_LOG(LogZhoenusPlayerState, Log, TEXT("Collision: %g - Shot: %g - Sight: %g -- Impact Bonus: %g"), DonutCollisionScore, DonutShotScore, DonutSightScore, impactBonus);
}

void AZhoenusPlayerState::RecordDonutShot()
{
	DonutShotScore += 1.f;
}

void AZhoenusPlayerState::ResetDonutEngagementStats()
{
	DonutCollisionScore = 0.f;
	DonutShotScore = 0.f;
	DonutSightScore = 0.f;
}
