// Copyright 2026 Jay Lauffer

#include "ZhoenusLobbyGameMode.h"

#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundWave.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/SoftObjectPath.h"
#include "ZhoenusLobbyPlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogZhoenusLobbyGameMode, Log, All);

AZhoenusLobbyGameMode::AZhoenusLobbyGameMode()
{
	{
		static ConstructorHelpers::FObjectFinder<USoundBase> LobbySong(TEXT("/Game/Sound/Lobby/LobbySong.LobbySong"));
		if (LobbySong.Object != nullptr)
		{
			LobbyMusicFallbackSound = LobbySong.Object;
		}
	}

	PlayerControllerClass = AZhoenusLobbyPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
}

void AZhoenusLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (bEnableLobbyMusic)
	{
		BuildLobbyMusicPlaylist();
		StartLobbyMusic();
	}
}

void AZhoenusLobbyGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopLobbyMusic(false);

	Super::EndPlay(EndPlayReason);
}

void AZhoenusLobbyGameMode::BuildLobbyMusicPlaylist()
{
	RuntimeLobbyMusicPlaylist.Reset();

	TSet<FSoftObjectPath> UniqueMusicPaths;
	auto AddMusicPath = [this, &UniqueMusicPaths](const FString& MusicPathString)
	{
		const FString NormalizedPath = NormalizeLobbyMusicObjectPath(MusicPathString);
		if (NormalizedPath.IsEmpty())
		{
			return;
		}

		const FSoftObjectPath MusicPath(NormalizedPath);
		if (!MusicPath.IsValid() || UniqueMusicPaths.Contains(MusicPath))
		{
			return;
		}

		UniqueMusicPaths.Add(MusicPath);
		RuntimeLobbyMusicPlaylist.Add(MusicPath);
	};

	AddMusicPath(LobbyMusicPath);
	for (const FString& MusicPath : LobbyMusicAssetPaths)
	{
		AddMusicPath(MusicPath);
	}

	if (bScanLobbyMusicDirectory)
	{
		TArray<FSoftObjectPath> ScannedMusicPaths;
		GatherLobbyMusicAssetPaths(ScannedMusicPaths);
		for (const FSoftObjectPath& MusicPath : ScannedMusicPaths)
		{
			AddMusicPath(MusicPath.ToString());
		}
	}

	UE_LOG(LogZhoenusLobbyGameMode, Log, TEXT("Prepared %d lobby music entries."), RuntimeLobbyMusicPlaylist.Num());
}

void AZhoenusLobbyGameMode::GatherLobbyMusicAssetPaths(TArray<FSoftObjectPath>& OutLobbyMusicAssetPaths) const
{
	if (LobbyMusicDirectory.IsEmpty())
	{
		return;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.PackagePaths.Add(*LobbyMusicDirectory);
	Filter.ClassPaths.Add(USoundWave::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;

	TArray<FAssetData> AssetData;
	AssetRegistryModule.Get().GetAssets(Filter, AssetData);
	AssetData.Sort([](const FAssetData& A, const FAssetData& B)
	{
		return A.AssetName.LexicalLess(B.AssetName);
	});

	for (const FAssetData& Asset : AssetData)
	{
		OutLobbyMusicAssetPaths.Add(Asset.GetSoftObjectPath());
	}
}

int32 AZhoenusLobbyGameMode::SelectLobbyMusicIndex() const
{
	if (RuntimeLobbyMusicPlaylist.Num() == 0)
	{
		return INDEX_NONE;
	}

	const int32 PlaylistCount = RuntimeLobbyMusicPlaylist.Num();
	int32 NextIndex = FMath::RandHelper(PlaylistCount);
	if (PlaylistCount > 1)
	{
		int32 SafetyCounter = 8;
		while (NextIndex == CurrentLobbyMusicIndex && SafetyCounter-- > 0)
		{
			NextIndex = FMath::RandHelper(PlaylistCount);
		}
	}

	return NextIndex;
}

USoundBase* AZhoenusLobbyGameMode::LoadLobbyMusicFromPath(const FSoftObjectPath& MusicPath) const
{
	if (!MusicPath.IsValid())
	{
		return nullptr;
	}

	return Cast<USoundBase>(MusicPath.TryLoad());
}

FString AZhoenusLobbyGameMode::NormalizeLobbyMusicObjectPath(const FString& MusicPath) const
{
	FString NormalizedPath = MusicPath.TrimStartAndEnd();
	if (NormalizedPath.IsEmpty())
	{
		return FString();
	}

	if (NormalizedPath.StartsWith(TEXT("/Game/")) && !NormalizedPath.Contains(TEXT(".")))
	{
		const FString AssetName = FPackageName::GetLongPackageAssetName(NormalizedPath);
		NormalizedPath = FString::Printf(TEXT("%s.%s"), *NormalizedPath, *AssetName);
	}

	return NormalizedPath;
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

	if (RuntimeLobbyMusicPlaylist.Num() == 0)
	{
		BuildLobbyMusicPlaylist();
	}

	USoundBase* LobbySound = nullptr;
	FString SelectedMusicLabel;
	const int32 FirstPlaylistIndex = SelectLobbyMusicIndex();
	if (FirstPlaylistIndex != INDEX_NONE)
	{
		for (int32 Offset = 0; Offset < RuntimeLobbyMusicPlaylist.Num(); ++Offset)
		{
			const int32 CandidateIndex = (FirstPlaylistIndex + Offset) % RuntimeLobbyMusicPlaylist.Num();
			const FSoftObjectPath& CandidatePath = RuntimeLobbyMusicPlaylist[CandidateIndex];
			LobbySound = LoadLobbyMusicFromPath(CandidatePath);
			if (LobbySound != nullptr)
			{
				CurrentLobbyMusicIndex = CandidateIndex;
				SelectedMusicLabel = CandidatePath.ToString();
				break;
			}

			UE_LOG(LogZhoenusLobbyGameMode, Warning, TEXT("Lobby music path %s could not be resolved."), *CandidatePath.ToString());
		}
	}

	if (LobbySound == nullptr)
	{
		LobbySound = LobbyMusicFallbackSound;
		SelectedMusicLabel = LobbySound ? LobbySound->GetPathName() : FString();
		CurrentLobbyMusicIndex = INDEX_NONE;
	}

	if (LobbySound == nullptr)
	{
		UE_LOG(LogZhoenusLobbyGameMode, Warning, TEXT("Lobby music could not be resolved from the configured playlist or bundled fallback."));
		return;
	}

	LobbyMusicComponent = UGameplayStatics::CreateSound2D(World, LobbySound, 1.0f, 1.0f, 0.0f, nullptr, false, false);
	if (!IsValid(LobbyMusicComponent))
	{
		UE_LOG(LogZhoenusLobbyGameMode, Warning, TEXT("Failed to create lobby music component for %s."), *SelectedMusicLabel);
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

	UE_LOG(LogZhoenusLobbyGameMode, Log, TEXT("Playing lobby music: %s"), *SelectedMusicLabel);
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
