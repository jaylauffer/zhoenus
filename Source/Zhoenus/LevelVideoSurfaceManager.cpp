#include "LevelVideoSurfaceManager.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "FileMediaSource.h"
#include "HAL/FileManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "MediaPlayer.h"
#include "MediaPlaylist.h"
#include "MediaSource.h"
#include "MediaTexture.h"
#include "Misc/Paths.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogLevelVideoSurfaceManager, Log, All);

namespace
{
	constexpr TCHAR SlateTextureParameterName[] = TEXT("SlateUI");
	constexpr TCHAR TintColorParameterName[] = TEXT("TintColorAndOpacity");
	constexpr TCHAR BackColorParameterName[] = TEXT("BackColor");
	constexpr TCHAR OpacityParameterName[] = TEXT("OpacityFromTexture");
}

ALevelVideoSurfaceManager::ALevelVideoSurfaceManager()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	TargetSurfaceNames = {
		TEXT("SM_MERGED_TemplateCube_Rounded_74"),
		TEXT("SM_MERGED_TemplateCube_Rounded_152"),
		TEXT("SM_MERGED_TemplateCube_Rounded_153"),
		TEXT("SM_MERGED_TemplateCube_Rounded"),
	};
	FallbackMediaSourcePath = TEXT("/Game/Movies/blend_fixed.blend_fixed");

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> VideoMaterialFinder(
		TEXT("/Engine/EngineMaterials/Widget3DPassThrough_Opaque_OneSided.Widget3DPassThrough_Opaque_OneSided"));
	if (VideoMaterialFinder.Succeeded())
	{
		VideoMaterialTemplate = VideoMaterialFinder.Object;
	}
}

void ALevelVideoSurfaceManager::BeginPlay()
{
	Super::BeginPlay();

	if (!bEnableLevelVideos)
	{
		UE_LOG(LogLevelVideoSurfaceManager, Log, TEXT("Level video playback disabled by config."));
		return;
	}

	if (!VideoMaterialTemplate)
	{
		UE_LOG(LogLevelVideoSurfaceManager, Warning, TEXT("Video material template is missing; skipping level video playback."));
		return;
	}

	BuildPlaylist();
	if (!MediaPlaylist || MediaPlaylist->Num() == 0)
	{
		UE_LOG(LogLevelVideoSurfaceManager, Warning, TEXT("No playable movies were found in Content/Movies."));
		return;
	}

	MediaPlayer = NewObject<UMediaPlayer>(this, TEXT("LevelVideoMediaPlayer"));
	MediaTexture = NewObject<UMediaTexture>(this, TEXT("LevelVideoMediaTexture"));
	if (!MediaPlayer || !MediaTexture)
	{
		UE_LOG(LogLevelVideoSurfaceManager, Warning, TEXT("Failed to allocate media playback objects."));
		return;
	}

	MediaPlayer->PlayOnOpen = true;
	MediaPlayer->SetLooping(false);
	MediaPlayer->OnEndReached.AddDynamic(this, &ALevelVideoSurfaceManager::HandleMediaEndReached);

	//MediaTexture->SetDefaultMediaPlayer(MediaPlayer); - note this builds on MacOS with XCode 26.2 UE 5.7.3 yet fails the in editor IOS build
	MediaTexture->SetMediaPlayer(MediaPlayer);
	MediaTexture->UpdateResource();

	ApplyVideoMaterialToTargets();
	if (!OpenRandomPlaylistEntry(false))
	{
		UE_LOG(LogLevelVideoSurfaceManager, Warning, TEXT("Failed to open a random level video."));
	}
}

void ALevelVideoSurfaceManager::HandleMediaEndReached()
{
	if (!bLoopRandomizedPlayback || !MediaPlaylist || MediaPlaylist->Num() == 0)
	{
		return;
	}

	if (!OpenRandomPlaylistEntry(true))
	{
		UE_LOG(LogLevelVideoSurfaceManager, Warning, TEXT("Failed to advance to the next randomized level video."));
	}
}

void ALevelVideoSurfaceManager::BuildPlaylist()
{
	MediaPlaylist = NewObject<UMediaPlaylist>(this, TEXT("LevelVideoPlaylist"));
	if (!MediaPlaylist)
	{
		return;
	}

	TSet<FString> UniqueMoviePaths;

	auto AddMoviePath = [this, &UniqueMoviePaths](const FString& Path)
	{
		if (Path.IsEmpty() || !FPaths::FileExists(Path))
		{
			return;
		}

		if (!UniqueMoviePaths.Contains(Path))
		{
			MediaPlaylist->AddFile(Path);
			UniqueMoviePaths.Add(Path);
		}
	};

	if (bScanMoviesDirectory)
	{
		TArray<FString> ScannedMoviePaths;
		GatherMoviePaths(ScannedMoviePaths);
		for (const FString& Path : ScannedMoviePaths)
		{
			AddMoviePath(Path);
		}
	}

	for (const FString& ConfiguredMovie : MovieFileNames)
	{
		FString ResolvedPath = ConfiguredMovie;
		if (!FPaths::FileExists(ResolvedPath))
		{
			ResolvedPath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Movies"), ConfiguredMovie);
		}
		AddMoviePath(ResolvedPath);
	}

	if (MediaPlaylist->Num() == 0 && !FallbackMediaSourcePath.IsEmpty())
	{
		if (UMediaSource* FallbackSource = LoadObject<UMediaSource>(nullptr, *FallbackMediaSourcePath))
		{
			MediaPlaylist->Add(FallbackSource);
		}
	}

	UE_LOG(LogLevelVideoSurfaceManager, Log, TEXT("Prepared %d movie entries for randomized level playback."), MediaPlaylist->Num());
}

void ALevelVideoSurfaceManager::GatherMoviePaths(TArray<FString>& OutMoviePaths) const
{
	const FString MoviesDirectory = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Movies"));
	TArray<FString> MovieFiles;
	IFileManager::Get().FindFiles(MovieFiles, *(MoviesDirectory / TEXT("*.mp4")), true, false);
	MovieFiles.Sort();

	for (const FString& MovieFile : MovieFiles)
	{
		OutMoviePaths.Add(FPaths::Combine(MoviesDirectory, MovieFile));
	}
}

void ALevelVideoSurfaceManager::ApplyVideoMaterialToTargets()
{
	if (!MediaTexture || !VideoMaterialTemplate)
	{
		return;
	}

	int32 AppliedSurfaceCount = 0;

	for (TActorIterator<AActor> ActorIt(GetWorld()); ActorIt; ++ActorIt)
	{
		AActor* Actor = *ActorIt;
		if (!IsValid(Actor))
		{
			continue;
		}

		TInlineComponentArray<UStaticMeshComponent*> MeshComponents;
		Actor->GetComponents(MeshComponents);

		for (UStaticMeshComponent* MeshComponent : MeshComponents)
		{
			if (!IsValid(MeshComponent) || !MatchesTargetSurface(*Actor, *MeshComponent))
			{
				continue;
			}

			const int32 MaterialCount = MeshComponent->GetNumMaterials();
			if (MaterialCount <= 0)
			{
				continue;
			}

			const int32 LastMaterialIndex = bApplyToAllMaterialSlots ? MaterialCount - 1 : 0;
			for (int32 MaterialIndex = 0; MaterialIndex <= LastMaterialIndex; ++MaterialIndex)
			{
				UMaterialInstanceDynamic* VideoMaterial = UMaterialInstanceDynamic::Create(VideoMaterialTemplate, this);
				if (!VideoMaterial)
				{
					continue;
				}

				VideoMaterial->SetTextureParameterValue(SlateTextureParameterName, MediaTexture);
				VideoMaterial->SetVectorParameterValue(TintColorParameterName, FLinearColor::White);
				VideoMaterial->SetVectorParameterValue(BackColorParameterName, FLinearColor::Black);
				VideoMaterial->SetScalarParameterValue(OpacityParameterName, 1.0f);
				MeshComponent->SetMaterial(MaterialIndex, VideoMaterial);
				AppliedMaterialInstances.Add(VideoMaterial);
			}

			++AppliedSurfaceCount;
			UE_LOG(LogLevelVideoSurfaceManager, Log, TEXT("Applied randomized video surface material to %s."), *Actor->GetName());
		}
	}

	if (AppliedSurfaceCount == 0)
	{
		UE_LOG(LogLevelVideoSurfaceManager, Warning, TEXT("No level surfaces matched the configured video targets. Tag a mesh actor with '%s' or adjust TargetSurfaceNames."), *TargetSurfaceTag.ToString());
	}
}

bool ALevelVideoSurfaceManager::MatchesTargetSurface(const AActor& Actor, const UStaticMeshComponent& MeshComponent) const
{
	if (TargetSurfaceTag != NAME_None && Actor.ActorHasTag(TargetSurfaceTag))
	{
		return true;
	}

	const FString ActorName = Actor.GetName();
	if (TargetSurfaceNames.ContainsByPredicate([&ActorName](const FString& Candidate)
		{
			return !Candidate.IsEmpty() && ActorName.Contains(Candidate, ESearchCase::IgnoreCase);
		}))
	{
		return true;
	}

	if (const UStaticMesh* StaticMesh = MeshComponent.GetStaticMesh())
	{
		const FString MeshName = StaticMesh->GetName();
		const FString MeshPath = StaticMesh->GetPathName();
		return TargetSurfaceNames.ContainsByPredicate([&MeshName, &MeshPath](const FString& Candidate)
			{
				return !Candidate.IsEmpty()
					&& (MeshName.Contains(Candidate, ESearchCase::IgnoreCase)
						|| MeshPath.Contains(Candidate, ESearchCase::IgnoreCase));
			});
	}

	return false;
}

bool ALevelVideoSurfaceManager::OpenRandomPlaylistEntry(const bool bAvoidCurrentSelection)
{
	if (!MediaPlayer || !MediaPlaylist || MediaPlaylist->Num() == 0)
	{
		return false;
	}

	const int32 PlaylistCount = MediaPlaylist->Num();
	int32 NextIndex = FMath::RandHelper(PlaylistCount);
	if (bAvoidCurrentSelection && PlaylistCount > 1)
	{
		int32 SafetyCounter = 8;
		while (NextIndex == CurrentPlaylistIndex && SafetyCounter-- > 0)
		{
			NextIndex = FMath::RandHelper(PlaylistCount);
		}
	}

	CurrentPlaylistIndex = NextIndex;
	return MediaPlayer->OpenPlaylistIndex(MediaPlaylist, CurrentPlaylistIndex);
}
