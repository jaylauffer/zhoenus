# Zhoenus

UE5 flying-action game project centered on the `SaveThemAll` loop: fly the ship, shepherd flyers through the goal, earn an end-of-run point payout, then reallocate or convert ship capability on the power-up screen.

## Public Snapshot Notes

- Large binary assets stay on an approved local LFS endpoint and are not served from the public remote.
- This repository keeps its committed `.lfsconfig` so internal contributors resolve large assets against the approved local endpoint.
- Public clones should expect code and metadata first; asset hydration still depends on local internal access.

## About EAB Integration and future directions
- [Philosophical/Technical GPT Chat about EAB integration, with instructions](https://chatgpt.com/share/69d93b17-85c0-8324-ac81-1ddef375123a)

## Current Reference Notes

- [Input and gamepad analysis](Docs/gamepad-controls-analysis.md)
- [Point system and convert flow](Docs/point-system-and-convert.md)
- [Donut flyer spawn analysis](Docs/donutflyer-spawn-analysis.md)
- [Donut flyer aggro design](Docs/donutflyer-aggro-design.md)
- [Donut flyer goal-lock analysis](Docs/donutflyer-goal-lock-analysis.md)
- [Gate clean-save design](Docs/gate-clean-save-design.md)
- [Reticle principles](Docs/reticle-principles.md)
- [Level-1 media playback](Docs/level1-media-playback.md)
- [SaveThemAll music playback](Docs/save-them-all-music-playback.md)
- [EAB + qcoin integration plan](Docs/eab-qcoin-integration-plan.md)
- [Game design concepts](Docs/game-design-concepts.md)

## Key Runtime Files

- `Source/Zhoenus/SaveThemAllGameMode.cpp`
- `Source/Zhoenus/SaveThemAllGameInstance.h`
- `Source/Zhoenus/SaveThemAllGameInstance.cpp`
- `Source/Zhoenus/SpaceshipPawn.cpp`
- `Source/Zhoenus/SaveThemAllPlayerController.cpp`

## Current Follow-Ups

- The Gate of Oblivion still needs a "clean save" collision pass. The art mesh can stay twisted and strange, but the gameplay collision should either reject or save, never trap. See `Docs/gate-clean-save-design.md`.
- `ShipEngineFlare` should eventually return to its original role as an attached aft-engine afterburner effect on the ship, with hue tied to thrust, rather than serving as a generic spawned flare fallback.
- The standalone `ShipFlareEmitter` asset in `Content/Effects/ShipFlareEmitter.uasset` is a Niagara emitter asset, not a Niagara system. Runtime code that wants to call `SpawnSystemAtLocation` must target a Niagara system wrapper instead.
- `PowerUpRoot`, `PowerUpScreenWidget`, `AdjustShip`, and `ConvertSpeed` now rely on native CommonUI wrappers for focus and controller navigation. The root focus handoff lives in `PowerUpRootUI`, and `PowerUpRoot` is now `auto_activate=true` because the `PowerUp` map adds that widget to the viewport but does not explicitly call `ActivateWidget()`.
- `ZhoenusButton` now depends on `ZhoenusRepeatButtonStyle` for visible selected/focused state, and that style asset now carries explicit rounded-box normal and selected brushes. If controller highlight regresses again, check the style asset before chasing navigation code.
- `PowerUpStatWidget` is now safely reparented to `UPowerUpStatWidgetUI`. The native row wrapper handles explicit left/right focus inside a stat row, while `AdjustShipUI` links those rows vertically and seeds them from the loaded `SaveThemAllGameInstance`.
- `AdjustShip` and `PowerUpStatWidget` are now stripped back to layout assets. Their old event-graph stat init and stat-value helper graphs were removed, so `UPowerUpStatWidgetUI` and `UAdjustShipUI` are the only active source of stat-row behavior and values.
- `UPowerUpStatWidgetUI` now owns the visible stat-row value directly instead of relying on blueprint click graphs. It also restores the row label from the blueprint `DisplayLabel` property so the layout asset still defines the human-readable stat name.
- `AdjustShip` and `Convert` are separate systems again. `AdjustShip` edits persistent handling stats such as `ForwardAcceleration`, `ReverseAcceleration`, `PitchAcceleration`, `YawAcceleration`, and `RollAcceleration`. `Convert` still operates on `MaxSpeed` and `MinSpeed`.
- `AdjustShip` native point accounting now also supports `MaxBatteryEnergy` and `BatteryRechargeRate`, but the current `AdjustShip` widget asset still needs matching row widgets before those battery adjustments become visible in-menu.
- The current default ship thrust baselines are `ForwardAcceleration = 500` and `ReverseAcceleration = 200`.
- `AdjustShip` now owns its point accounting in native code through `USaveThemAllGameInstance`. The current rule is intentionally symmetric relative to the saved build you opened the screen with, and it uses fractional per-unit cost ratios rather than a flat `1:1` spend. The current temporary ratio table is: `ForwardAcceleration = 0.32`, `ReverseAcceleration = 0.18`, `PitchAcceleration = 0.07`, `YawAcceleration = 0.10`, and `RollAcceleration = 0.08`.
- The `PowerUp`, `Convert`, and `AdjustShip` menu color scheme still needs a contrast/accessibility pass. Current contrast is weaker than it should be for some players; this matters, but reticle readability remains the higher-priority visual issue.
- `USaveThemAllGameInstance::LoadGame` now repairs legacy saves that predate `ReverseAcceleration`. Those older saves deserialize that field as zero, so the loader normalizes it back to the baseline reverse thrust value instead of presenting a broken stat row on fresh launch.
- Debug point grants are now available through the shared player-controller console command `GrantPowerPoints <amount>` in non-shipping builds only. It adds directly to `USaveThemAllGameInstance::points`, mirrors the grant into `AcquiredPoints`, and saves immediately so the `PowerUp` screens can be tested without playing a full round.
- Project source control is intentionally disabled. `Config/DefaultSourceControlSettings.ini` sets `Provider=None`, and the local Mac editor cache mirrors that in `Saved/Config/MacEditor/SourceControlSettings.ini` to avoid Perforce startup noise on this machine.
- `Level-1` can now randomize in-world movie playback from `Content/Movies` at runtime through `ALevelVideoSurfaceManager`, which is spawned by `ASaveThemAllGameMode`. It swaps tagged or named static-mesh surfaces over to a media-backed material and chooses a random clip on start and after each clip ends.
- `SaveThemAll` music no longer depends on editing the `LevelSong` MetaSound graph to change the playable set. `ASaveThemAllGameMode` now builds a runtime playlist from configured `/Game/Sound` asset paths plus optional directory scanning, then plays one track for the run and still advances to `PowerUp` when that track ends.

## Portable Project Copy

For Linux or Raspberry Pi work, use `Script/sync_portable_project.sh <destination>` instead of a raw `rsync -aE`.

The helper keeps the rebuildable project tree and excludes generated/editor plus Apple-platform residue:

- `Intermediate/`
- `Saved/`
- `DerivedDataCache/`
- `Binaries/`
- `Build/Mac/`
- `Build/IOS/`
- `IOS/`
- `Zhoenus (Mac).xcworkspace/`
- `Zhoenus (IOS).xcworkspace/`
- `Zhoenus (TVOS).xcworkspace/`
- `Zhoenus (VisionOS).xcworkspace/`
- `.DS_Store` and `._*`

It uses plain `rsync -a` rather than `-aE`, so copies to ExFAT or Linux targets do not accumulate AppleDouble sidecar files. If the destination is deployment-only rather than a working clone, add `--exclude='.git/'`.
