// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SaveThemAllGameMode.h"
#include "SaveThemAllGameState.h"
#include "ZhoenusPlayerController.h"
#include "ZhoenusPlayerState.h"
#include "ZhoenusPawn.h"
#include "Goal.h"
#include "MassSpawner.h"

DEFINE_LOG_CATEGORY_STATIC(LogSaveThemAllGameMode, Log, All);

ASaveThemAllGameMode::ASaveThemAllGameMode()
{
	//PlayerControllerClass = AZhoenusPlayerController::StaticClass();
	// set default pawn class to our flying pawn
	DefaultPawnClass = AZhoenusPawn::StaticClass();
	PlayerStateClass = AZhoenusPlayerState::StaticClass();
	GameStateClass = ASaveThemAllGameState::StaticClass();
}

void ASaveThemAllGameMode::Score(AGoal* goal, APawn* player, APawn* ball)
{
	++GetGameState<ASaveThemAllGameState>()->Saved;
	ball->Destroy();
}
