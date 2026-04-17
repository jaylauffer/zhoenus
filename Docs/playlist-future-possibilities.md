# SaveThemAll Playlist Future Possibilities

Date: 2026-04-11

## Purpose

This note captures the intended future direction for `SaveThemAll` music sources.

It is not a commitment to build every provider listed here. It is a guardrail
document so future playlist work supports the game instead of adding account
friction or breaking offline single-player.

## Core Rule

Offline single-player must always work.

That means:

- the game must be able to start and finish a run with no network connection
- the game must always have a locally available playable music source
- remote playlist providers are optional enhancements, not runtime requirements

The gameplay loop stays:

- `Startup.umap`
- `Level-1.umap`
- `PowerUp.umap`

Music selection should support that loop, not interrupt it.

## Current Direction

The current packaged-song path remains the foundation.

Today the game can:

- build a runtime playlist from configured `/Game/Sound/Game` asset paths
- optionally scan `/Game/Sound/Game` for additional `SoundWave` assets
- play one song for the run
- advance to `PowerUp` when that song ends

That should remain the guaranteed fallback path even if more advanced providers
are added later.

## Future Source Types

The long-term music system should allow different playlist providers to feed the
same gameplay-facing playlist model.

Possible sources:

- packaged Unreal audio assets bundled with the game
- iOS device-local music playlists
- Android device-local media libraries if a clean supported path exists
- network playlist providers such as SoundCloud
- other future sources only if they can fit the offline-first rule

## Preferred Design Shape

Gameplay code should not know about SoundCloud, iTunes, Spotify, or any other
provider-specific API.

Instead, a music source should resolve into a common playlist result.

One reasonable future shape:

```cpp
struct FPlayableTrack
{
    FString StableId;
    FString Title;
    FString Artist;
    FString ArtworkUrl;
    FString StreamUrl;
    float DurationSeconds = 0.f;
    bool bRequiresNetwork = false;
};

struct FResolvedPlaylist
{
    FString SourceId;
    FString DisplayName;
    TArray<FPlayableTrack> Tracks;
    bool bOfflineSafe = false;
};
```

Possible provider classes:

- `LocalPackagedProvider`
- `IOSDeviceLibraryProvider`
- `AndroidDeviceLibraryProvider`
- `SoundCloudProvider`

`ASaveThemAllGameMode` should consume resolved playlists, not provider-specific
logic.

## Selection Priority

The intended priority is:

1. local packaged fallback always exists
2. if a chosen provider resolves a usable playlist quickly, use it
3. if that provider fails, fall back to the packaged playlist

That means:

- local packaged music is the safety net
- device-local and network providers are enhancements
- the game should never block run start on remote playlist resolution

## iOS Direction

iOS is a natural candidate for a device-local playlist source.

The aspiration is:

- use an available on-device playlist as a music source
- resolve the selected playlist into playable entries
- keep the packaged Unreal playlist available as fallback

This matches the offline-first rule better than a network-only source.

## Android Direction

Android may eventually support either:

- a device-local media source
- or a network playlist provider

If Android uses a network-backed source, the same offline-first fallback rule
still applies. A run must not depend on a successful live network lookup.

## SoundCloud Direction

SoundCloud is a plausible future network playlist provider because its developer
API exposes playlist and stream information that can be normalized into a game
playlist.

A future `SoundCloudProvider` would likely do this:

1. authenticate with SoundCloud
2. resolve a playlist identifier or URL
3. fetch playlist metadata and track list
4. resolve stream information for each track
5. build a normalized `FResolvedPlaylist`
6. hand that playlist to the game

Important guardrails:

- SoundCloud should be treated as network-only
- it should not replace the packaged fallback
- metadata caching is useful
- persistent offline storage of fetched user audio should not be assumed

## Spotify Direction

Spotify may be attractive from a user perspective, but it should be treated as a
more constrained integration candidate than SoundCloud.

Design caution:

- Spotify typically wants playback handled through Spotify-controlled SDK flows
  rather than handing out general-purpose stream URLs
- that makes it a weaker fit for a game-owned playback pipeline

So Spotify should remain a later investigation, not the default assumed remote
provider.

## Ordering And Playlist Growth

If the game continues using the widening-pool song selection model, playlist
order matters.

Design implication:

- the curated front of the playlist should stay intentionally ordered
- later discovered or appended songs should be added in a stable predictable way
- provider-specific playlist order should be preserved when that order carries
  user meaning

If playlist order is unstable, widening-pool behavior becomes hard to reason
about.

## Packaging Rule

If songs are intended to be playable offline on a platform, they must be
explicitly included in the packaged build through a reliable cook path.

Do not assume:

- a config string by itself guarantees cook inclusion
- a network provider can stand in for packaged music on mobile

If mobile launches with bundled songs, that bundle should be treated as a real
platform deliverable, not a temporary accident of editor packaging.

## Implementation Order

Recommended order for future work:

1. keep the current packaged-song path healthy
2. stabilize playlist ordering and widening behavior
3. introduce a provider abstraction for music sources
4. add one device-local provider first
5. add one remote provider only after fallback behavior is solid
6. keep run start resilient even when provider resolution fails

## Non-Goals

This document does not assume:

- account login is required to play the game
- cloud music should replace packaged fallback audio
- remote providers should block run start
- per-track commerce, DRM work, or streaming-service entitlement flows should
  become part of the core `SaveThemAll` loop

## External Notes

These public docs are relevant if SoundCloud or platform playlist work is later
implemented:

- SoundCloud API introduction: https://developers.soundcloud.com/docs/api/introduction
- SoundCloud API reference: https://developers.soundcloud.com/docs/api/reference
- SoundCloud streaming URL update: https://developers.soundcloud.com/blog/api-streaming-urls
- SoundCloud URN migration note: https://developers.soundcloud.com/blog/urn-num-to-string/
- SoundCloud API rate limits: https://developers.soundcloud.com/docs/api/rate-limits.html
- SoundCloud API terms: https://developers.soundcloud.com/docs/api/terms-of-use

These links are here as design context, not as a signal that SoundCloud
integration is already underway in the runtime code.
