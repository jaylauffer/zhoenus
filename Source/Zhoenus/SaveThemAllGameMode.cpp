//Copyright Jay Lauffer 2026

#include "SaveThemAllGameMode.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "SaveThemAllGameState.h"
#include "SaveThemAllPlayerController.h"
#include "SaveThemAllGameInstance.h"
#include "ZhoenusPlayerState.h"
#include "ZhoenusPawn.h"
#include "DonutFlyerPawn.h"
#include "DonutFlyerSpawner.h"
#include "Goal.h"
#include "LevelVideoSurfaceManager.h"
#include "SpaceshipHUD.h"
#include "EngineUtils.h"
#include "Math/UnrealMathUtility.h"
#include "CoreFwd.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h" 
#include "Engine/LevelBounds.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "NiagaraCommon.h"
#include "NiagaraComponent.h"
#include "NiagaraComponentSettings.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundWave.h"
#include "TimerManager.h"

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT   // ----------- EDITOR / DEV ONLY ----------
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogSaveThemAllGameMode, Log, All);

ASaveThemAllGameMode::ASaveThemAllGameMode()
{
	{
		static ConstructorHelpers::FObjectFinder<USoundBase> Song(TEXT("/Game/Sound/Game/LevelSong.LevelSong"));
		if (Song.Object != nullptr)
		{
			LegacyFallbackSong = Song.Object;
		}
	}

	PlayerControllerClass = ASaveThemAllPlayerController::StaticClass();
	// set default pawn class to our flying pawn
	DefaultPawnClass = AZhoenusPawn::StaticClass();
	PlayerStateClass = AZhoenusPlayerState::StaticClass();
	GameStateClass = ASaveThemAllGameState::StaticClass();
	HUDClass = ASpaceshipHUD::StaticClass();

    BuildSongPlaylist();
}

void ASaveThemAllGameMode::BeginPlay()
{
	Super::BeginPlay();

	auto gi{ GetGameInstance<USaveThemAllGameInstance>() };
	if (gi && !gi->LoadGame(TEXT("SaveSlot01"), 0))
	{
		gi->MakeNewGame();
		gi->SaveGame();
	}

	UWorld* w{ GetWorld() };

	GetWorldTimerManager().SetTimerForNextTick(this, &ASaveThemAllGameMode::RefreshInitialDonutTotal);

	const int64 TotalAttempts = gi ? gi->TotalAttempts : 0;
	StartSongForRun(TotalAttempts);

	if (w)
	{
		LevelVideoSurfaceManager = w->SpawnActor<ALevelVideoSurfaceManager>();
	}
}

void ASaveThemAllGameMode::BuildSongPlaylist()
{
	RuntimeSongPlaylist.Reset();

	TSet<FSoftObjectPath> UniqueSongPaths;
	auto AddSongPath = [this, &UniqueSongPaths](const FString& SongPathString)
	{
		const FString NormalizedPath = NormalizeSongObjectPath(SongPathString);
		if (NormalizedPath.IsEmpty())
		{
			return;
		}

		const FSoftObjectPath SongPath(NormalizedPath);
		if (!SongPath.IsValid() || UniqueSongPaths.Contains(SongPath))
		{
			return;
		}
        UE_LOG(LogSaveThemAllGameMode, Log, TEXT("Found music path %s for SaveThemAll."), *NormalizedPath);
		UniqueSongPaths.Add(SongPath);
	};

	for (const FString& SongPath : SongAssetPaths)
	{
		AddSongPath(SongPath);
	}
    
	if (bScanSoundDirectory)
	{
		TArray<FSoftObjectPath> ScannedSongPaths;
		GatherSongAssetPaths(ScannedSongPaths);
		for (const FSoftObjectPath& SongPath : ScannedSongPaths)
		{
			AddSongPath(SongPath.ToString());
		}
	}

    for (const FString& MandatorySong : FirstThreeSongPaths)
    {
        if(UniqueSongPaths.Remove(MandatorySong) > 0)
        {
            RuntimeSongPlaylist.Add(MandatorySong);
        }
        else
        {
            UE_LOG(LogSaveThemAllGameMode, Error, TEXT("Required song %s missing."), *MandatorySong);
        }
    }
    
    for (const FSoftObjectPath& SongPath : UniqueSongPaths)
    {
        RuntimeSongPlaylist.Add(SongPath);
    }
    
	UE_LOG(LogSaveThemAllGameMode, Log, TEXT("Prepared %d runtime music entries for SaveThemAll."), RuntimeSongPlaylist.Num());
}

void ASaveThemAllGameMode::GatherSongAssetPaths(TArray<FSoftObjectPath>& OutSongAssetPaths) const
{
	if (SoundAssetDirectory.IsEmpty())
	{
		return;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.PackagePaths.Add(*SoundAssetDirectory);
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
		OutSongAssetPaths.Add(Asset.GetSoftObjectPath());
	}
}

int32 ASaveThemAllGameMode::SelectSongIndex(const int64 TotalAttempts) const
{
	if (RuntimeSongPlaylist.Num() == 0)
	{
		return INDEX_NONE;
	}

	const int32 PlaylistCount = RuntimeSongPlaylist.Num();
	const int32 PinnedSongCount = FMath::Min(FirstThreeSongPaths.Num(), PlaylistCount);

	// The first few runs are intentionally fixed to the curated front of the playlist.
	if (PinnedSongCount > 0 && TotalAttempts < PinnedSongCount)
	{
		const int32 FixedIndex = static_cast<int32>(TotalAttempts);
		UE_LOG(LogSaveThemAllGameMode, Log, TEXT("Pinned early-run song index %d selected for attempt %lld."), FixedIndex, TotalAttempts);
		return FixedIndex;
	}

	// After the pinned opening runs, widen the random pool by one playlist entry per attempt
	// until the full runtime playlist becomes available.
	const int32 UnlockedSongCount = static_cast<int32>(
		FMath::Clamp<int64>(TotalAttempts + 1, 1, PlaylistCount));
	return FMath::RandRange(0, UnlockedSongCount - 1);
}

int32 ASaveThemAllGameMode::SelectLegacySongWaveIndex(const int64 TotalAttempts) const
{
	const int32 LegacySongCount = FMath::Max(1, SongAssetPaths.Num());
    const int32 ChoiceCount = static_cast<int32>(TotalAttempts % LegacySongCount + 1);
    return FMath::RandRange(0, ChoiceCount - 1);
}

USoundBase* ASaveThemAllGameMode::LoadSongFromPath(const FSoftObjectPath& SongPath) const
{
	if (!SongPath.IsValid())
	{
		return nullptr;
	}

	return Cast<USoundBase>(SongPath.TryLoad());
}

bool ASaveThemAllGameMode::StartSongForRun(const int64 TotalAttempts)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	USoundBase* SelectedSong = nullptr;
	FString SelectedSongLabel;
	bool bUsingLegacyLevelSongFallback = false;

	if (bEnableRuntimeSongPlaylist && RuntimeSongPlaylist.Num() > 0)
	{
		const int32 SongIndex = SelectSongIndex(TotalAttempts);
		if (SongIndex != INDEX_NONE)
		{
			SelectedSong = LoadSongFromPath(RuntimeSongPlaylist[SongIndex]);
			SelectedSongLabel = RuntimeSongPlaylist[SongIndex].ToString();
            UE_LOG(LogSaveThemAllGameMode, Error, TEXT("3-peat Number of attempts %d, Index %d, Selected song: %s"), TotalAttempts, SongIndex, *SelectedSongLabel);
		}
	}

	const FString NormalizedFallbackPath = NormalizeSongObjectPath(FallbackSongPath);
	if (SelectedSong == nullptr && !NormalizedFallbackPath.IsEmpty())
	{
		SelectedSong = LoadSongFromPath(FSoftObjectPath(NormalizedFallbackPath));
		SelectedSongLabel = NormalizedFallbackPath;
		bUsingLegacyLevelSongFallback = SelectedSong != nullptr && NormalizedFallbackPath.Contains(TEXT("LevelSong"));
	}

	if (SelectedSong == nullptr)
	{
		SelectedSong = LegacyFallbackSong;
		SelectedSongLabel = SelectedSong ? SelectedSong->GetPathName() : FString();
		bUsingLegacyLevelSongFallback = SelectedSong == LegacyFallbackSong && SelectedSongLabel.Contains(TEXT("LevelSong"));
	}

	if (SelectedSong == nullptr)
	{
		UE_LOG(LogSaveThemAllGameMode, Warning, TEXT("SaveThemAll could not resolve any song to play."));
		return false;
	}

	BackgroundMusicComponent = UGameplayStatics::CreateSound2D(World, SelectedSong);
	if (BackgroundMusicComponent == nullptr)
	{
		UE_LOG(LogSaveThemAllGameMode, Warning, TEXT("Failed to create background music component for %s."), *SelectedSongLabel);
		return false;
	}

	if (bUsingLegacyLevelSongFallback)
	{
		BackgroundMusicComponent->SetIntParameter(TEXT("WaveIndex"), SelectLegacySongWaveIndex(TotalAttempts));
	}

	BackgroundMusicComponent->OnAudioFinished.AddDynamic(this, &ASaveThemAllGameMode::OnSongFinished);
	BackgroundMusicComponent->Play();

	UE_LOG(LogSaveThemAllGameMode, Log, TEXT("Playing SaveThemAll song: %s"), *SelectedSongLabel);
	return true;
}

FString ASaveThemAllGameMode::NormalizeSongObjectPath(const FString& SongPath) const
{
	FString NormalizedPath = SongPath.TrimStartAndEnd();
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

void ASaveThemAllGameMode::RefreshInitialDonutTotal()
{
	UWorld* World = GetWorld();
	ASaveThemAllGameState* State = GetGameState<ASaveThemAllGameState>();
	if (World == nullptr || State == nullptr)
	{
		return;
	}

	int32 RequestedFlyerCount = 0;
	for (TActorIterator<ADonutFlyerSpawner> SpawnerIt{ World }; SpawnerIt; ++SpawnerIt)
	{
		RequestedFlyerCount += SpawnerIt->SpawnAmount;
	}

	int32 ActualFlyerCount = 0;
	for (TActorIterator<ADonutFlyerPawn> DonutIt{ World }; DonutIt; ++DonutIt)
	{
		if (IsValid(*DonutIt) && !DonutIt->IsActorBeingDestroyed())
		{
			++ActualFlyerCount;
		}
	}

	State->Total = ActualFlyerCount;
	UE_LOG(LogSaveThemAllGameMode, Log, TEXT("SaveThemAll initial donuts: requested %d, spawned %d."), RequestedFlyerCount, ActualFlyerCount);

	if (ActualFlyerCount != RequestedFlyerCount)
	{
		UE_LOG(LogSaveThemAllGameMode, Warning, TEXT("Requested donut count does not match actual spawn count at level start."));
	}
}

void ASaveThemAllGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	USaveThemAllGameInstance* gi{ GetGameInstance<USaveThemAllGameInstance>() };
	ASaveThemAllGameState* gs{ GetGameState<ASaveThemAllGameState>() };
	float newPoints{ 0.f };
	if (gi && gs)
	{
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
	}
	Super::EndPlay(EndPlayReason);
}

void ASaveThemAllGameMode::OnSongFinished()
{
	UGameplayStatics::OpenLevel(GetWorld(), "PowerUp");
}

void ASaveThemAllGameMode::Score(AGoal* goal, APawn* player, APawn* ball)
{
	ASaveThemAllGameState* SaveThemAllGameState = GetGameState<ASaveThemAllGameState>();
	if (SaveThemAllGameState)
	{
		++SaveThemAllGameState->Saved;
	}

	ASpaceshipPawn* spaceship{ Cast<ASpaceshipPawn>(player) };
	ADonutFlyerPawn* donut{ Cast<ADonutFlyerPawn>(ball) };
	if (spaceship)
	{
		spaceship->MaxSpeed += 5.f;
		spaceship->MinSpeed -= 2.f;
	}
	
	bool destroyDonut{ true };
	UNiagaraSystem* disintegrate{ LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Effects/DonutDissolve.DonutDissolve"), nullptr, LOAD_None, nullptr) };
	if (disintegrate)
	{
		UNiagaraComponent* nc{ UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), disintegrate, ball->GetActorLocation(), ball->GetActorRotation(), ball->GetActorScale()) };
		if (nc)
		{
			if (donut && donut->GetPlaneMesh() && donut->GetPlaneMesh()->GetStaticMesh())
			{
				// The Niagara system exposes the mesh input as a user parameter, including
				// the legacy typo preserved in the asset graph. Set both known names so the
				// dissolve system receives the live donut mesh instead of its generic fallback.
				UStaticMesh* DonutMesh = donut->GetPlaneMesh()->GetStaticMesh();
				nc->SetVariableStaticMesh(TEXT("User.New Stativc Mesh"), DonutMesh);
				nc->SetVariableStaticMesh(TEXT("User.Static Mesh"), DonutMesh);
			}
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
