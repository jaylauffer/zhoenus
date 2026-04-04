// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "ZhoenusGameMode.h"


#if WITH_EDITOR || UE_BUILD_DEVELOPMENT   // ----------- EDITOR / DEV ONLY ----------
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
/**  For future reference, we may want something more like this..
void USaveThemAllGameMode::GetAvailableSongPaths(TArray<FSoftObjectPath>& OutPaths) const
{
    // Example: grab every asset that derives from UMediaSource
    FAssetRegistryModule& RegModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    IAssetRegistry& Reg = RegModule.Get();

    TArray<FAssetData> MediaAssets;
    Reg.GetAssetsByClass(UMediaSource::StaticClass()->GetClassPathName(), MediaAssets);

    for (const FAssetData& Asset : MediaAssets)
    {
        OutPaths.Add(Asset.GetSoftObjectPath());
    }
}
#else                                           // ----------- SHIPPING / MOBILE ------------
void USaveThemAllGameMode::GetAvailableSongPaths(TArray<FSoftObjectPath>& OutPaths) const
{
    // Temporary placeholder – an empty list so the game still runs.
    // Later you can fill this from a JSON file, a remote server, or iOS Media Library.
    OutPaths.Empty();
} */
#endif

#include "SaveThemAllGameMode.generated.h"

class USoundBase;
class UAudioComponent;
class UNiagaraSystem;
class ALevelVideoSurfaceManager;

UCLASS(Config=Game, MinimalAPI)
class ASaveThemAllGameMode : public AZhoenusGameMode
{
	GENERATED_BODY()

public:
	ASaveThemAllGameMode();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Score(AGoal *goal, APawn *player, APawn *ball) override;
	

	UFUNCTION()
	virtual void OnSongFinished();

private:
	void RefreshInitialDonutTotal();
	void BuildSongPlaylist();
	void GatherSongAssetPaths(TArray<FSoftObjectPath>& OutSongAssetPaths) const;
	int32 SelectSongIndex(int64 TotalAttempts) const;
	int32 SelectLegacySongWaveIndex(int64 TotalAttempts) const;
	USoundBase* LoadSongFromPath(const FSoftObjectPath& SongPath) const;
	bool StartSongForRun(int64 TotalAttempts);
	FString NormalizeSongObjectPath(const FString& SongPath) const;

	UPROPERTY(EditAnywhere, Config, Category = "Music")
	bool bEnableRuntimeSongPlaylist = true;

	UPROPERTY(EditAnywhere, Config, Category = "Music")
	bool bScanSoundDirectory = true;

	UPROPERTY(EditAnywhere, Config, Category = "Music")
	FString SoundAssetDirectory = TEXT("/Game/Sound");

	UPROPERTY(EditAnywhere, Config, Category = "Music")
	TArray<FString> SongAssetPaths;

	UPROPERTY(EditAnywhere, Config, Category = "Music")
	FString PreferredFirstSongPath = TEXT("/Game/Sound/20220506-ode-to-ts-mono.20220506-ode-to-ts-mono");

	UPROPERTY(EditAnywhere, Config, Category = "Music")
	FString FallbackSongPath = TEXT("/Game/Sound/LevelSong.LevelSong");

	UPROPERTY(Transient)
	TObjectPtr<ALevelVideoSurfaceManager> LevelVideoSurfaceManager;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BackgroundMusicComponent;

	UPROPERTY(Transient)
	TArray<FSoftObjectPath> RuntimeSongPlaylist;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> LegacyFallbackSong;
};
