# SaveThemAll Music Playback

## Purpose

`SaveThemAll` uses one song per run.

The song is not just background flavor. When the selected track ends,
`ASaveThemAllGameMode::OnSongFinished()` advances the run to the `PowerUp` map.

This path should stay easy to curate without forcing every playlist change
through the `LevelSong` MetaSound asset.

## Runtime Path

- `ASaveThemAllGameMode::BeginPlay()` calls `BuildSongPlaylist()`
- the game mode resolves a runtime playlist from configured song asset paths
- if enabled, it also scans `/Game/Sound` for additional `SoundWave` assets
- it chooses one song for the run
- `UGameplayStatics::CreateSound2D()` plays that song
- when the song finishes, `OnSongFinished()` opens `PowerUp`

Relevant source files:

- `Source/Zhoenus/SaveThemAllGameMode.h`
- `Source/Zhoenus/SaveThemAllGameMode.cpp`
- `Config/DefaultGame.ini`

## Current Configuration

Runtime playlist config lives under `[/Script/Zhoenus.SaveThemAllGameMode]`
in `Config/DefaultGame.ini`.

Current knobs:

- `bEnableRuntimeSongPlaylist=True`
- `bScanSoundDirectory=True`
- `SoundAssetDirectory=/Game/Sound`
- `PreferredFirstSongPath=/Game/Sound/20220506-ode-to-ts-mono.20220506-ode-to-ts-mono`
- `FallbackSongPath=/Game/Sound/LevelSong.LevelSong`
- `+SongAssetPaths=...` for the curated front of the playlist

Meaning:

- `SongAssetPaths` defines the stable front of the playlist and therefore the
  early-attempt bias
- `PreferredFirstSongPath` preserves the "first run gets Ode to TS" behavior
- `bScanSoundDirectory=True` appends any newly imported `SoundWave` assets from
  `/Game/Sound` without requiring MetaSound graph edits
- `FallbackSongPath` keeps the old `LevelSong` asset available if the runtime
  playlist resolves to nothing
- if that fallback `LevelSong` MetaSound is used, the game mode still drives its
  `WaveIndex` parameter from the same attempt-based unlock logic

## Design Rules

- Keep this tied to the `SaveThemAll` loop: one track per run, then advance.
- Prefer config and folder management over editing a MetaSound graph just to add
  or reorder songs.
- Treat `/Game/Sound` as the music library for this mode unless the project
  later creates a dedicated subfolder.
- If curated order matters, change `SongAssetPaths` in config.
- If automatic discovery matters more, leave scanning enabled and drop new songs
  into `/Game/Sound`.

## Non-Goals

This system does not currently try to:

- crossfade between songs
- loop endlessly during a run
- treat music as a surface-projected media system like `Level-1` video
- replace imported Unreal audio assets with raw external file playback

The current job is simpler: make the playable song set easy to manage while
preserving the run-end transition behavior.
