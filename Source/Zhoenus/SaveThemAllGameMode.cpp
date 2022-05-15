// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SaveThemAllGameMode.h"
#include "SaveThemAllGameState.h"
#include "ZhoenusPlayerController.h"
#include "ZhoenusPlayerState.h"
#include "ZhoenusPawn.h"
#include "DonutFlyerPawn.h"
#include "DonutFlyerSpawner.h"
#include "Goal.h"
#include "SpaceshipHUD.h"
#include "EngineUtils.h"
#include "Math/UnrealMathUtility.h"
#include "CoreFwd.h"
#include "Engine/LevelBounds.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogSaveThemAllGameMode, Log, All);

ASaveThemAllGameMode::ASaveThemAllGameMode()
{
	//PlayerControllerClass = AZhoenusPlayerController::StaticClass();
	// set default pawn class to our flying pawn
	DefaultPawnClass = AZhoenusPawn::StaticClass();
	PlayerStateClass = AZhoenusPlayerState::StaticClass();
	GameStateClass = ASaveThemAllGameState::StaticClass();
	HUDClass = ASpaceshipHUD::StaticClass();
}

void ASaveThemAllGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* w{ GetWorld() };
	ASaveThemAllGameState* state{ GetGameState<ASaveThemAllGameState>() };
	for (TActorIterator<ADonutFlyerSpawner> ActorItr{ w }; ActorItr; ++ActorItr)
	{
		state->Total += ActorItr->SpawnAmount;
	}
	////FBox box{ALevelBounds::CalculateLevelBounds(w->GetCurrentLevel())};
	//FBox box{ FVector{-18000, -18000, 100}, FVector{18000, 18000, 12000} };
	//for (int i = 0; i < 100; ++i)
	//{
	//	FVector spawn{ FMath::RandPointInBox(box) };
	//	FRotator rot{ };
	//	w->SpawnActor<ADonutFlyerPawn>(spawn, rot);
	//}
}

void ASaveThemAllGameMode::Score(AGoal* goal, APawn* player, APawn* ball)
{
	++GetGameState<ASaveThemAllGameState>()->Saved;
	ASpaceshipPawn* spaceship{ Cast<ASpaceshipPawn>(player) };
	spaceship->MaxSpeed += 5.f;
	spaceship->MinSpeed -= 2.f;
	ball->Destroy();
}
