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

## Runtime asset path

Lobby source files:

- `Music/Lobby/*`

Current imported lobby runtime assets:

- `/Game/Sound/Lobby/*`

Current curated active lobby asset:

- `/Game/Sound/Lobby/LobbySong.LobbySong`

## Current behavior target

While the player is in lobby context:

1. Play `LobbySong`
2. Use approximately `42%` volume
3. Fade in when playback starts
4. Allow the song to finish
5. Wait a random silence interval between `16` and `42` seconds
6. Play `LobbySong` again

When leaving lobby context:

1. Fade out the lobby music
2. Do not let it continue into gameplay music handling

## Explicit non-goals

This prototype does not yet try to:

- support multiple lobby tracks with shuffle logic
- make lobby music participate in gameplay run completion logic
- replace the gameplay playlist or `SaveThemAll` run-ending transition

## Validation target

- `Startup.umap` plays the lobby music behavior
- `PowerUp.umap` plays the lobby music behavior
- `Level-1.umap` does not use the lobby track
- entering gameplay resolves to `SaveThemAllV1`, not the lobby game mode
- gameplay still transitions to `PowerUp` when the gameplay song ends
