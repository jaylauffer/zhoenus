# Zhoenus Lobby Music Design

Date: 2026-04-18

## Purpose

Define the current intended runtime behavior for menu-facing music before more
audio code lands.

This note exists so Zhoenus keeps a clear distinction between:

- gameplay-run music
- lobby/menu music

## Lobby mode

Zhoenus now treats the menu shell as a dedicated lobby game mode concern.

Current project state:

- `GlobalDefaultGameMode=/Script/Zhoenus.ZhoenusLobbyGameMode`
- `GameDefaultMap=/Game/Map/Startup.Startup`
- `TransitionMap=/Game/Map/PowerUp.PowerUp`
- `Level-*` maps are routed back to gameplay through
  `GameModeMapPrefixes -> SaveThemAllV1`

For the current prototype, lobby mode is intended for:

- `Startup.umap`
- `PowerUp.umap`

Gameplay context means:

- `Level-1.umap`

## Ownership

Lobby music should be owned by `AZhoenusLobbyGameMode`, not by
`USaveThemAllGameInstance`.

Why:

- `ASaveThemAllGameMode` owns gameplay-run music and the "song ends ->
  `PowerUp`" transition
- lobby maps should not inherit that gameplay-ending behavior
- menu-shell music is a map-mode concern, not save-state or persistent stat
  ownership
- this keeps `USaveThemAllGameInstance` focused on persistent progression and
  ship state

## Relationship To Gameplay Music

`ASaveThemAllGameMode` has similar runtime music plumbing because it already
builds a gameplay playlist from configured `/Game/Sound/Game` assets and
scanned cooked sound assets. That overlap is real, but the two game modes should
not be collapsed into one playback policy.

Shared helper code is acceptable for asset-resolution mechanics:

- path normalization
- `AssetRegistry` sound scanning
- deterministic asset sorting
- de-duplicated soft-object playlist construction
- soft-path loading and fallback attempts

Mode-specific behavior must remain in the owning game mode:

- `ASaveThemAllGameMode` keeps run-aware song selection, early-run curated
  tracks, widening random pools, and the "song ends -> `PowerUp`" transition
- `AZhoenusLobbyGameMode` keeps lobby randomization, previous-track avoidance,
  fade in/out, random silence delay, and replay without ending a gameplay run

Proceed by extracting only the shared resolver mechanics if the duplication
starts to slow development. Do not move the gameplay or lobby playback lifecycle
into `USaveThemAllGameInstance` as part of this slice.

## Runtime asset path

Lobby source files:

- `Music/Lobby/*`

Current imported lobby runtime assets:

- `/Game/Sound/Lobby/*`

Current curated active lobby assets:

- `/Game/Sound/Lobby/LobbySong.LobbySong`
- `/Game/Sound/Lobby/Tropical-Delight-Menus-BGM.Tropical-Delight-Menus-BGM`

Packaging note:

- lobby music is resolved at runtime, so cooked builds must include
  `/Game/Sound/Lobby`
- the project now also keeps a bundled `LobbySong` fallback in
  `AZhoenusLobbyGameMode` so device builds do not depend solely on a config
  string resolving at runtime

## Current behavior target

While the player is in lobby context:

1. Build a lobby playlist from configured `/Game/Sound/Lobby` assets and the
   cooked lobby music directory.
2. Use approximately `42%` volume
3. Fade in when playback starts
4. Allow the song to finish
5. Wait a random silence interval between `16` and `42` seconds
6. Play another lobby playlist entry, avoiding the previous track when possible

When leaving lobby context:

1. Fade out the lobby music
2. Do not let it continue into gameplay music handling

## Explicit non-goals

This prototype does not yet try to:

- make lobby music participate in gameplay run completion logic
- replace the gameplay playlist or `SaveThemAll` run-ending transition
- play raw source files from `Music/Lobby` without importing them as Unreal
  audio assets first

## Validation target

- `Startup.umap` plays the lobby playlist behavior
- `PowerUp.umap` plays the lobby playlist behavior
- `Level-1.umap` does not use the lobby track
- entering gameplay resolves to `SaveThemAllV1`, not the lobby game mode
- gameplay still transitions to `PowerUp` when the gameplay song ends
