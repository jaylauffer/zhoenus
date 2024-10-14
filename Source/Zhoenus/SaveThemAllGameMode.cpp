// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SaveThemAllGameMode.h"
#include "SaveThemAllGameState.h"
#include "SaveThemAllPlayerController.h"
#include "SaveThemAllGameInstance.h"
#include "ZhoenusPlayerState.h"
#include "ZhoenusPawn.h"
#include "DonutFlyerPawn.h"
#include "DonutFlyerSpawner.h"
#include "Goal.h"
#include "SpaceshipHUD.h"
#include "EngineUtils.h"
#include "Math/UnrealMathUtility.h"
#include "CoreFwd.h"
#include "Components/AudioComponent.h"
#include "Engine/LevelBounds.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraCommon.h"
#include "NiagaraComponent.h"
#include "NiagaraComponentSettings.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"



DEFINE_LOG_CATEGORY_STATIC(LogSaveThemAllGameMode, Log, All);

ASaveThemAllGameMode::ASaveThemAllGameMode()
{
	{
		static ConstructorHelpers::FObjectFinder<USoundBase> Song(TEXT("/Game/Sound/LevelSong"));
		if (Song.Object != nullptr)
		{
			song = Song.Object;

		}

	}

	PlayerControllerClass = ASaveThemAllPlayerController::StaticClass();
	// set default pawn class to our flying pawn
	DefaultPawnClass = AZhoenusPawn::StaticClass();
	PlayerStateClass = AZhoenusPlayerState::StaticClass();
	GameStateClass = ASaveThemAllGameState::StaticClass();
	HUDClass = ASpaceshipHUD::StaticClass();

}

void ASaveThemAllGameMode::BeginPlay()
{
	Super::BeginPlay();

	auto gi{ GetGameInstance<USaveThemAllGameInstance>() };
#if WITH_EDITOR
	gi->LoadGame("SaveSlot01", 0);
#endif

	UWorld* w{ GetWorld() };

	ASaveThemAllGameState* state{ GetGameState<ASaveThemAllGameState>() };
	for (TActorIterator<ADonutFlyerSpawner> ActorItr{ w }; ActorItr; ++ActorItr)
	{
		state->Total += ActorItr->SpawnAmount;
	}

	UAudioComponent* bgm{ UGameplayStatics::CreateSound2D(w, song) };
	if (bgm)
	{
		//Index 0 is Ode to Taylor Swift, we want the first time a player plays that they get Ode to TS
		//with this code the first time players will get ode to ts, and until
		//the total attempts pass 15, they will tend towards the lower indexes (indices) 
		int32 index{ static_cast<int32>(FMath::RandHelper(16) % (gi->TotalAttempts + 1)) };
		bgm->SetIntParameter("WaveIndex", index);
		bgm->OnAudioFinished.AddDynamic(this, &ASaveThemAllGameMode::OnSongFinished);
		bgm->Play();
	}
}

void ASaveThemAllGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	USaveThemAllGameInstance* gi{ GetGameInstance<USaveThemAllGameInstance>() };
	ASaveThemAllGameState* gs{ GetGameState<ASaveThemAllGameState>() };
	float newPoints{ 0.f };
	if (gs->Saved < gs->Total)
	{
		float percentageSaved{ float(gs->Saved) / float(gs->Total) };
		if (percentageSaved > 0.74f)
		{
			newPoints = float(gs->Saved) * 1.25f;
		}
		else if (percentageSaved > 0.3f)
		{
			newPoints = float(gs->Saved) * 1.f;
		}
		else
		{
			newPoints = float(gs->Saved) * .9f;
		}
	}
	else
	{
		newPoints = float(gs->Saved) * 1.45f;
	}
	gi->points += newPoints;
	gi->AcquiredPoints += newPoints;
	gi->TotalAttempts++;
	gi->SaveGame();
	Super::EndPlay(EndPlayReason);
}

void ASaveThemAllGameMode::OnSongFinished()
{
	UGameplayStatics::OpenLevel(GetWorld(), "PowerUp");
}

void ASaveThemAllGameMode::Score(AGoal* goal, APawn* player, APawn* ball)
{
	++GetGameState<ASaveThemAllGameState>()->Saved;
	ASpaceshipPawn* spaceship{ Cast<ASpaceshipPawn>(player) };
	ADonutFlyerPawn* donut{ Cast<ADonutFlyerPawn>(ball) };
	spaceship->MaxSpeed += 5.f;
	spaceship->MinSpeed -= 2.f;
	
	bool destroyDonut{ true };
	UNiagaraSystem* disintegrate{ LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Effects/DonutDissolve.DonutDissolve"), nullptr, LOAD_None, nullptr) };
	if (disintegrate)
	{
		UNiagaraComponent* nc{ UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), disintegrate, ball->GetActorLocation(), ball->GetActorRotation(), ball->GetActorScale()) };
		if (nc)
		{
			nc->SetVariableStaticMesh("StativcMesh", donut->GetPlaneMesh()->GetStaticMesh());
			nc->OnSystemFinished.AddDynamic(donut, &ADonutFlyerPawn::DelayedDestroy);
			ball->SetActorHiddenInGame(true);
			ball->SetActorEnableCollision(false);
			ball->SetActorTickEnabled(false);
			destroyDonut = false;
		}
	}
	if (destroyDonut)
	{
		ball->Destroy();
	}
}
