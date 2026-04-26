// Copyright 2026 Jay Lauffer

#pragma once

#include "CoreMinimal.h"
#include "ZhoenusGameMode.h"
#include "ZhoenusLobbyGameMode.generated.h"

class UAudioComponent;
class USoundBase;

UCLASS(Config=Game)
class AZhoenusLobbyGameMode : public AZhoenusGameMode
{
	GENERATED_BODY()

public:
	AZhoenusLobbyGameMode();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	USoundBase* ResolveLobbyMusic();
	void StartLobbyMusic();
	void StopLobbyMusic(bool bFadeOut);
	void ScheduleLobbyMusicReplay();
	void CancelLobbyMusicTimers();
	void DestroyLobbyMusicComponent();

	UFUNCTION()
	void HandleLobbyMusicFinished();

	UFUNCTION()
	void HandleLobbyMusicReplayTimerElapsed();

	UFUNCTION()
	void HandleLobbyMusicFadeOutTimerElapsed();

	UPROPERTY(EditAnywhere, Config, Category = "Music|Lobby")
	bool bEnableLobbyMusic = true;

	UPROPERTY(EditAnywhere, Config, Category = "Music|Lobby")
	FString LobbyMusicPath = TEXT("/Game/Sound/Lobby/LobbySong.LobbySong");

	UPROPERTY(EditAnywhere, Config, Category = "Music|Lobby", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LobbyMusicVolumeMultiplier = 0.42f;

	UPROPERTY(EditAnywhere, Config, Category = "Music|Lobby", meta = (ClampMin = "0.0"))
	float LobbyMusicFadeInSeconds = 1.5f;

	UPROPERTY(EditAnywhere, Config, Category = "Music|Lobby", meta = (ClampMin = "0.0"))
	float LobbyMusicFadeOutSeconds = 1.5f;

	UPROPERTY(EditAnywhere, Config, Category = "Music|Lobby", meta = (ClampMin = "0.0"))
	float LobbyMusicReplayDelayMinSeconds = 16.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Music|Lobby", meta = (ClampMin = "0.0"))
	float LobbyMusicReplayDelayMaxSeconds = 42.0f;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> LobbyMusicComponent;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> LobbyMusicSound;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> LobbyMusicFallbackSound;

	FTimerHandle LobbyMusicReplayTimerHandle;
	FTimerHandle LobbyMusicFadeOutTimerHandle;
};
