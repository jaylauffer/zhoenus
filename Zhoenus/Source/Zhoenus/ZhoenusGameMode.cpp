// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ZhoenusGameMode.h"
#include "ZhoenusPlayerController.h"
#include "ZhoenusPawn.h"

AZhoenusGameMode::AZhoenusGameMode()
{
	PlayerControllerClass = AZhoenusPlayerController::StaticClass();
	// set default pawn class to our flying pawn
	DefaultPawnClass = AZhoenusPawn::StaticClass();
}
