// Copyright 2018 loadngo Games, All rights reserved


#include "ZhoenusPlayerState.h"

DEFINE_LOG_CATEGORY(LogZhoenusPlayerState)

AZhoenusPlayerState::AZhoenusPlayerState()
{
	BumpAggro = 0.f;
	ShotAggro = 0.f;
	SeenAggro = 0.f;
}

void AZhoenusPlayerState::OnDonutHitFromMe(float impactMagnitude)
{
	float impactBonus{ 5.f * impactMagnitude / 750.f };
	BumpAggro += impactBonus;
	UE_LOG(LogZhoenusPlayerState, Log, TEXT("Bump: %g - Shot: %g - Seen: %g -- Impact Bonus: %g"), BumpAggro, ShotAggro, SeenAggro, impactBonus);
}

void AZhoenusPlayerState::OnDonutShootedFromMe()
{
	ShotAggro += 1.f;
}

void AZhoenusPlayerState::ClearPlayerState()
{
	BumpAggro = 0.f;
	ShotAggro = 0.f;
	SeenAggro = 0.f;
}
