// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ZhoenusSoccerGameMode.h"
#include "ZhoenusSoccerGameState.h"
#include "ZhoenusPlayerController.h"
#include "ZhoenusPlayerState.h"
#include "ZhoenusPawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogZhoenusSoccerGameMode, Log, All);

AZhoenusSoccerGameMode::AZhoenusSoccerGameMode()
{
	//PlayerControllerClass = AZhoenusPlayerController::StaticClass();
	// set default pawn class to our flying pawn
	DefaultPawnClass = AZhoenusPawn::StaticClass();
	PlayerStateClass = AZhoenusPlayerState::StaticClass();
	GameStateClass = AZhoenusSoccerGameState::StaticClass();
}


void AZhoenusSoccerGameMode::IncrementScore(int32 index, int32 amount)
{
	AZhoenusSoccerGameState* state = GetGameState<AZhoenusSoccerGameState>();
	if (IsValid(state) && index < state->Score.Num())
	{
		state->Score[index] += amount;
	}
}

void AZhoenusSoccerGameMode::ClearAfterGoal()
{
	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator it = World->GetPlayerControllerIterator(); it; ++it)
		{
			AZhoenusPlayerState* ps{ (*it)->GetPlayerState<AZhoenusPlayerState>() };
			UE_LOG(LogZhoenusSoccerGameMode, Log, TEXT("Clear Player %s - Bump: %g Shot: %g Seen: %g"), *(*it)->GetName(), ps->BumpAggro, ps->ShotAggro, ps->SeenAggro);
			ps->ClearPlayerState();
			if (AZhoenusPawn *zp = Cast<AZhoenusPawn>((*it)->GetPawn()))
			{
				zp->IsDonutTarget = false;
			}
		}
	}
}
