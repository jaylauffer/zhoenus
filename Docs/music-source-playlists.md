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

## Immediate content intent

The current `Music/Lobby` folder contains:

- `Tropical-Delight-Menus-BGM.wav`

Immediate behavior target:

- `LobbySong` should act as the background music for menu screens until the
  lobby playlist grows beyond a single curated track

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

Until the lobby playlist grows into multiple curated tracks, the intended lobby
behavior is:

- use `/Game/Sound/Lobby/LobbySong.LobbySong`
- play at approximately `42%` volume
- when the song ends, wait a random interval between `16` and `42` seconds
- then play it again
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

## Non-goals

This note does not yet decide:

- whether lobby playback lives in a game instance, startup map actor, UI
  widget, or dedicated audio manager in the final architecture
- whether multiple lobby tracks should shuffle, rotate, or remain fixed

Those are implementation decisions to make after the playlist intent is clear.

## Short version

For current prototype work:

- `Music/Game` feeds the gameplay playlist
- `Music/Lobby` feeds the lobby playlist
- `/Game/Sound/Lobby/LobbySong.LobbySong` is the current intended lobby/menu
  runtime track
- menu screens should use lobby music
- `Level-1` should keep using the gameplay playlist model
