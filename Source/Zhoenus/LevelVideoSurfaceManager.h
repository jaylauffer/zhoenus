#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelVideoSurfaceManager.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UMediaPlayer;
class UMediaPlaylist;
class UMediaTexture;
class UStaticMeshComponent;

UCLASS(Config = Game)
class ZHOENUS_API ALevelVideoSurfaceManager : public AActor
{
	GENERATED_BODY()

public:
	ALevelVideoSurfaceManager();

	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, Config, Category = "Video")
	bool bEnableLevelVideos = true;

	UPROPERTY(EditAnywhere, Config, Category = "Video")
	bool bScanMoviesDirectory = true;

	UPROPERTY(EditAnywhere, Config, Category = "Video")
	TArray<FString> MovieFileNames;

	UPROPERTY(EditAnywhere, Config, Category = "Video")
	TArray<FString> TargetSurfaceNames;

	UPROPERTY(EditAnywhere, Config, Category = "Video")
	FName TargetSurfaceTag = TEXT("VideoSurface");

	UPROPERTY(EditAnywhere, Config, Category = "Video")
	bool bApplyToAllMaterialSlots = true;

	UPROPERTY(EditAnywhere, Config, Category = "Video")
	bool bLoopRandomizedPlayback = true;

	UPROPERTY(EditAnywhere, Config, Category = "Video")
	FString FallbackMediaSourcePath;

	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> MediaPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UMediaTexture> MediaTexture;

	UPROPERTY(Transient)
	TObjectPtr<UMediaPlaylist> MediaPlaylist;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> AppliedMaterialInstances;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> VideoMaterialTemplate;

	UFUNCTION()
	void HandleMediaEndReached();

private:
	void BuildPlaylist();
	void GatherMoviePaths(TArray<FString>& OutMoviePaths) const;
	void ApplyVideoMaterialToTargets();
	bool MatchesTargetSurface(const AActor& Actor, const UStaticMeshComponent& MeshComponent) const;
	bool OpenRandomPlaylistEntry(bool bAvoidCurrentSelection);

	int32 CurrentPlaylistIndex = INDEX_NONE;
};
