// Copyright 2026 Jay Lauffer

#include "ZhoenusGameMode.h"
#include "ZhoenusPlayerController.h"
#include "ZhoenusPlayerState.h"
#include "ZhoenusPawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogZhoenusGameMode, Log, All);

AZhoenusGameMode::AZhoenusGameMode()
{
	PlayerControllerClass = AZhoenusPlayerController::StaticClass();
	// set default pawn class to our flying pawn
	//DefaultPawnClass = AZhoenusPawn::StaticClass();
	//PlayerStateClass = AZhoenusPlayerState::StaticClass();
}

