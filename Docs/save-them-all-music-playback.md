# SaveThemAll Music Playback

## Purpose

`SaveThemAll` uses one song per run.

Companion note:

- [Zhoenus Music Source Playlists](music-source-playlists.md)
- [Zhoenus Lobby Music Design](lobby-music-design.md)

The song is not just background flavor. When the selected track ends,
`ASaveThemAllGameMode::OnSongFinished()` advances the run to the `PowerUp` map.

This path should stay easy to curate without forcing every playlist change
through the `LevelSong` MetaSound asset.

## Runtime Path

- `ASaveThemAllGameMode::BeginPlay()` calls `BuildSongPlaylist()`
- the game mode resolves a runtime playlist from configured song asset paths
- if enabled, it also scans `/Game/Sound/Game` for additional `SoundWave` assets
- it chooses one song for the run
- `UGameplayStatics::CreateSound2D()` plays that song
- when the song finishes, `OnSongFinished()` opens `PowerUp`

Relevant source files:

- `Source/Zhoenus/SaveThemAllGameMode.h`
- `Source/Zhoenus/SaveThemAllGameMode.cpp`
- `Source/Zhoenus/ZhoenusLobbyGameMode.h`
- `Source/Zhoenus/ZhoenusLobbyGameMode.cpp`
- `Config/DefaultGame.ini`

## Current Configuration

Runtime playlist config lives under `[/Script/Zhoenus.SaveThemAllGameMode]`
in `Config/DefaultGame.ini`.

Current knobs:

- `bEnableRuntimeSongPlaylist=True`
- `bScanSoundDirectory=True`
- `SoundAssetDirectory=/Game/Sound/Game`
- `FallbackSongPath=/Game/Sound/Game/LevelSong.LevelSong`
- `+SongAssetPaths=...` for the curated front of the playlist

Meaning:

- `SongAssetPaths` defines the curated front of the runtime playlist
- the first few runs are pinned to the curated opening songs before the random
  pool gradually widens across the playlist
- `bScanSoundDirectory=True` appends any newly imported `SoundWave` assets from
  `/Game/Sound/Game` without requiring MetaSound graph edits
- `FallbackSongPath` keeps the old `LevelSong` asset available if the runtime
  playlist resolves to nothing
- if that fallback `LevelSong` MetaSound is used, the game mode still drives its
  `WaveIndex` parameter from the same attempt-based unlock logic

## Design Rules

- Keep this tied to the `SaveThemAll` loop: one track per run, then advance.
- Prefer config and folder management over editing a MetaSound graph just to add
  or reorder songs.
- Treat `/Game/Sound/Game` as the gameplay music library for this mode.
- Treat this as the **gameplay** playlist path, not the menu/lobby playlist.
- Treat lobby music as a separate system owned by `AZhoenusLobbyGameMode`.
- Raw source files placed under `Music/Game` are intended to feed this gameplay
  playlist after they are imported into `/Game/Sound/Game`.
- If curated order matters, change `SongAssetPaths` in config.
- If automatic discovery matters more, leave scanning enabled and drop new songs
  into `/Game/Sound/Game`.

## Non-Goals

This system does not currently try to:

- crossfade between songs
- loop endlessly during a run
- treat music as a surface-projected media system like `Level-1` video
- replace imported Unreal audio assets with raw external file playback

The current job is simpler: make the playable song set easy to manage while
preserving the run-end transition behavior.
