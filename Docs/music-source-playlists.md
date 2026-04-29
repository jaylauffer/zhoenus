# Zhoenus Music Source Playlists

Date: 2026-04-17

## Purpose

Define the intended source-of-truth for Zhoenus music intake before more menu
or gameplay playback work lands.

This note is about playlist intent and file organization, not the final
implementation details of every playback component.

## Core split

Zhoenus should treat music in two categories:

- `Music/Game`
- `Music/Lobby`

Those folders are source folders for playlist curation.

They do **not** imply that the shipping game should read arbitrary raw files
from disk at runtime. The current Unreal runtime should continue to rely on
packaged/cooked audio assets or another deliberate playback path.

## Intended mapping

### 1. Game playlist

Source:

- `Music/Game/*`

Purpose:

- music intended for the active gameplay loop
- tracks that can drive a `Level-1` session
- tracks whose playback rules must stay compatible with the current
  `SaveThemAll` song-ended transition to `PowerUp`

Current runtime relationship:

- this maps onto the existing `SaveThemAll` runtime playlist concept
- imported gameplay assets now live under `/Game/Sound/Game`
- gameplay code should keep consuming a resolved game playlist, not raw
  provider-specific or folder-specific logic

### 2. Lobby playlist

Source:

- `Music/Lobby/*`

Purpose:

- music intended for menu-facing screens
- startup/menu ambience
- post-run or upgrade/menu shell ambience

Current immediate target:

- when Zhoenus is on menu screens, use the lobby playlist rather than the
  gameplay playlist
- the current menu-facing maps are:
  - `Startup.umap`
  - `PowerUp.umap`
- widget-only menu states hosted inside `PowerUp.umap` should still be treated
  as lobby/menu context, not gameplay context
- imported lobby assets now live under `/Game/Sound/Lobby`
- the current curated runtime lobby asset is
  `/Game/Sound/Lobby/LobbySong.LobbySong`
- additional imported lobby assets are resolved from `/Game/Sound/Lobby` by
  `AZhoenusLobbyGameMode`
- cooked builds must explicitly include `/Game/Sound/Game` and
  `/Game/Sound/Lobby`, because both gameplay and lobby music are selected at
  runtime from project state rather than only from hard map references

## Immediate content intent

The current `Music/Lobby` folder contains:

- `MainOutput_2026-04-29_08-19-37.wav`
- `Tropical-Delight-Menus-BGM.wav`
- `Tropical-Delight-Savory-Recipe.wav`
- `Working-for-a-living_2026-04-28_07-14-02_Enhanced.wav`

Current imported lobby runtime assets:

- `/Game/Sound/Lobby/LobbySong.LobbySong`
- `/Game/Sound/Lobby/Tropical-Delight-Menus-BGM.Tropical-Delight-Menus-BGM`

Immediate behavior target:

- menu screens should build a lobby playlist from imported `/Game/Sound/Lobby`
  assets
- raw files in `Music/Lobby` are source material and must be imported before
  they can play in cooked iOS builds

## Lobby context rule

Zhoenus does **not** currently have a separate dedicated "lobby gameplay mode"
class.

Today:

- `GlobalDefaultGameMode` now points at `ZhoenusLobbyGameMode`
- `Startup.umap` and `PowerUp.umap` resolve to lobby context through that lobby
  game mode
- `Level-1.umap` remains gameplay context

`Level-*` gameplay maps are routed back to `SaveThemAllV1` with a map-prefix
override, so lobby ownership stays separate from the gameplay run loop.

## Current lobby playback target

The intended lobby behavior is:

- use the imported `/Game/Sound/Lobby` assets
- play at approximately `42%` volume
- when a song ends, wait a random interval between `16` and `42` seconds
- then play another lobby track, avoiding an immediate repeat when possible
- fade in on playback start
- fade out when leaving lobby context

## Design rules

1. The game playlist and lobby playlist are distinct concerns.
2. Menu playback should not consume the same playlist-selection logic used for
   the `Level-1` run loop.
3. The gameplay loop still owns the "song ends -> `PowerUp`" rule.
4. Lobby playback should be allowed to loop or persist across menu navigation
   without pretending it is a gameplay session track.
5. New music source files should be placed by intent:
   - gameplay track source -> `Music/Game`
   - menu/lobby track source -> `Music/Lobby`
6. Runtime code should resolve playlists from curated project state, not treat
   incoming folder contents as implicit authority with no documentation.
7. Lobby behavior should be owned by `AZhoenusLobbyGameMode`, not by the save
   game instance.

## Implementation Direction

The current `ASaveThemAllGameMode` and staged `AZhoenusLobbyGameMode` music
work intentionally have different playback policies, and they should not keep
duplicating low-level playlist plumbing forever.

Keep separate:

- gameplay song selection policy in `ASaveThemAllGameMode`
- lobby song selection policy in `AZhoenusLobbyGameMode`
- gameplay song-ended behavior, including the `PowerUp` transition
- lobby song-ended behavior, including fade-out, silence delay, and replay

Factor later:

- normalizing `/Game/...` package paths into object paths
- scanning a cooked sound asset directory through `AssetRegistry`
- sorting discovered `USoundWave` assets deterministically
- building a de-duplicated `FSoftObjectPath` playlist from configured and
  scanned sources
- loading `USoundBase` from a soft object path with clear fallback behavior

A good next refactor target is a small shared helper such as
`ZhoenusSoundPlaylist` or `ZhoenusMusicPlaylistResolver`. It should return
resolved playlist data and loadable sound assets, not decide when music starts,
what volume/fade policy to use, what index should be selected, or what should
happen when playback finishes.

This keeps the project DRY at the asset-resolution layer while preserving the
important gameplay/menu split at the mode-behavior layer.

## Non-goals

This note does not yet decide:

- whether lobby playback lives in a game instance, startup map actor, UI
  widget, or dedicated audio manager in the final architecture
- whether lobby playback should eventually support crossfades, weighted
  rotation, or richer playlist curation

Those are implementation decisions to make after the playlist intent is clear.

## Short version

For current prototype work:

- `Music/Game` feeds the gameplay playlist
- `Music/Lobby` feeds the lobby playlist
- imported `/Game/Sound/Lobby` assets are the current intended lobby/menu
  runtime playlist
- menu screens should use lobby music
- `Level-1` should keep using the gameplay playlist model
