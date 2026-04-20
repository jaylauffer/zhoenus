// Copyright 2026 Jay Lauffer

#include "ZhoenusLobbyGameMode.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"

DEFINE_LOG_CATEGORY_STATIC(LogZhoenusLobbyGameMode, Log, All);

AZhoenusLobbyGameMode::AZhoenusLobbyGameMode()
{
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
}

void AZhoenusLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (bEnableLobbyMusic)
	{
		StartLobbyMusic();
	}
}

void AZhoenusLobbyGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopLobbyMusic(false);

	Super::EndPlay(EndPlayReason);
}

USoundBase* AZhoenusLobbyGameMode::ResolveLobbyMusic()
{
	if (LobbyMusicPath.IsEmpty())
	{
		return nullptr;
	}

	if (IsValid(LobbyMusicSound))
	{
		return LobbyMusicSound;
	}

	const FSoftObjectPath LobbyMusicAssetPath(LobbyMusicPath);
	LobbyMusicSound = Cast<USoundBase>(LobbyMusicAssetPath.TryLoad());
	return LobbyMusicSound;
}

void AZhoenusLobbyGameMode::StartLobbyMusic()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	CancelLobbyMusicTimers();
	DestroyLobbyMusicComponent();

	USoundBase* LobbySound = ResolveLobbyMusic();
	if (LobbySound == nullptr)
	{
		UE_LOG(LogZhoenusLobbyGameMode, Warning, TEXT("Lobby music path %s could not be resolved."), *LobbyMusicPath);
		return;
	}

	LobbyMusicComponent = UGameplayStatics::CreateSound2D(World, LobbySound, 1.0f, 1.0f, 0.0f, nullptr, false, false);
	if (!IsValid(LobbyMusicComponent))
	{
		UE_LOG(LogZhoenusLobbyGameMode, Warning, TEXT("Failed to create lobby music component for %s."), *LobbyMusicPath);
		return;
	}

	LobbyMusicComponent->bAutoDestroy = false;
	LobbyMusicComponent->OnAudioFinished.AddDynamic(this, &AZhoenusLobbyGameMode::HandleLobbyMusicFinished);

	if (LobbyMusicFadeInSeconds > 0.0f)
	{
		LobbyMusicComponent->FadeIn(LobbyMusicFadeInSeconds, LobbyMusicVolumeMultiplier);
	}
	else
	{
		LobbyMusicComponent->SetVolumeMultiplier(LobbyMusicVolumeMultiplier);
		LobbyMusicComponent->Play();
	}

	const float SoundDuration = LobbySound->GetDuration();
	if (LobbyMusicFadeOutSeconds > 0.0f && SoundDuration > LobbyMusicFadeOutSeconds)
	{
		World->GetTimerManager().SetTimer(
			LobbyMusicFadeOutTimerHandle,
			this,
			&AZhoenusLobbyGameMode::HandleLobbyMusicFadeOutTimerElapsed,
			SoundDuration - LobbyMusicFadeOutSeconds,
			false);
	}
}

void AZhoenusLobbyGameMode::StopLobbyMusic(bool bFadeOut)
{
	CancelLobbyMusicTimers();

	if (!IsValid(LobbyMusicComponent))
	{
		return;
	}

	if (bFadeOut && LobbyMusicFadeOutSeconds > 0.0f && LobbyMusicComponent->IsPlaying())
	{
		LobbyMusicComponent->FadeOut(LobbyMusicFadeOutSeconds, 0.0f);
		return;
	}

	LobbyMusicComponent->Stop();
	DestroyLobbyMusicComponent();
}

void AZhoenusLobbyGameMode::ScheduleLobbyMusicReplay()
{
	UWorld* World = GetWorld();
	if (World == nullptr || !bEnableLobbyMusic)
	{
		return;
	}

	const float MinDelay = FMath::Max(0.0f, LobbyMusicReplayDelayMinSeconds);
	const float MaxDelay = FMath::Max(MinDelay, LobbyMusicReplayDelayMaxSeconds);
	const float ReplayDelay = FMath::FRandRange(MinDelay, MaxDelay);

	World->GetTimerManager().SetTimer(
		LobbyMusicReplayTimerHandle,
		this,
		&AZhoenusLobbyGameMode::HandleLobbyMusicReplayTimerElapsed,
		ReplayDelay,
		false);
}

void AZhoenusLobbyGameMode::CancelLobbyMusicTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LobbyMusicReplayTimerHandle);
		World->GetTimerManager().ClearTimer(LobbyMusicFadeOutTimerHandle);
	}

	LobbyMusicReplayTimerHandle.Invalidate();
	LobbyMusicFadeOutTimerHandle.Invalidate();
}

void AZhoenusLobbyGameMode::DestroyLobbyMusicComponent()
{
	if (!IsValid(LobbyMusicComponent))
	{
		LobbyMusicComponent = nullptr;
		return;
	}

	LobbyMusicComponent->OnAudioFinished.RemoveDynamic(this, &AZhoenusLobbyGameMode::HandleLobbyMusicFinished);
	LobbyMusicComponent->DestroyComponent();
	LobbyMusicComponent = nullptr;
}

void AZhoenusLobbyGameMode::HandleLobbyMusicFinished()
{
	DestroyLobbyMusicComponent();
	ScheduleLobbyMusicReplay();
}

void AZhoenusLobbyGameMode::HandleLobbyMusicReplayTimerElapsed()
{
	StartLobbyMusic();
}

void AZhoenusLobbyGameMode::HandleLobbyMusicFadeOutTimerElapsed()
{
	if (!IsValid(LobbyMusicComponent) || !LobbyMusicComponent->IsPlaying())
	{
		return;
	}

	LobbyMusicComponent->FadeOut(LobbyMusicFadeOutSeconds, 0.0f);
}
