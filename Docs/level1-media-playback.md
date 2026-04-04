# Level-1 Media Playback

## Purpose

`Level-1` can project `.mp4` content from `Content/Movies` onto selected static-mesh
surfaces at runtime.

This path exists for environmental screens and decorative in-world playback, not
for startup movies.

## Runtime Path

- `ASaveThemAllGameMode::BeginPlay()` spawns `ALevelVideoSurfaceManager`
- `ALevelVideoSurfaceManager` scans `Content/Movies` for `.mp4` files
- it builds a runtime playlist
- it picks a random clip on startup
- when a clip ends, it picks another random clip
- it applies a media-backed material to matching static-mesh surfaces

Relevant source files:

- `Source/Zhoenus/LevelVideoSurfaceManager.h`
- `Source/Zhoenus/LevelVideoSurfaceManager.cpp`
- `Source/Zhoenus/SaveThemAllGameMode.cpp`

## Surface Selection

The manager matches surfaces in this order:

1. any actor tagged `VideoSurface`
2. actor names containing configured `TargetSurfaceNames`
3. static-mesh asset names or paths containing configured `TargetSurfaceNames`

Current config lives in `Config/DefaultGame.ini`:

- `TargetSurfaceTag=VideoSurface`
- `TargetSurfaceNames=SM_MERGED_TemplateCube_Rounded_74`
- `TargetSurfaceNames=SM_MERGED_TemplateCube_Rounded_152`
- `TargetSurfaceNames=SM_MERGED_TemplateCube_Rounded_153`
- `TargetSurfaceNames=SM_MERGED_TemplateCube_Rounded`

If the intended blue metallic screen is not one of those objects, tag the desired
mesh actor `VideoSurface` in the editor instead of hardcoding a new guess in C++.

## Playback Notes

- Playback uses UE5 Media Framework, not startup-movie settings.
- The manager uses `Widget3DPassThrough_Opaque_OneSided` as the runtime material
  template and feeds it a transient `UMediaTexture`.
- `Content/Movies/blend_fixed.uasset` remains the fallback `FileMediaSource` if no
  `.mp4` scan succeeds.

## Goose Guidance

If Goose or another agent touches this system:

- verify the relevant surface match first
- do not replace this with startup-movie logic
- prefer config or actor tags over map-specific hardcoding
- preserve randomized playback unless the task explicitly asks for a fixed clip
