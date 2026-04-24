# Level-1 Landscape Boundary Note

Date: 2026-04-25

## Purpose

Document the current `Level-1` landscape-containment problem before choosing a
fix.

This note is about the present gameplay situation and the evidence currently
available from source and map inspection. It is not yet a decision document for
which containment approach should ship.

## Gameplay Behavior Under Discussion

`Level-1` is the live `SaveThemAll` gameplay map.

The intended loop is:

- fly the ship in readable 6DOF space
- gather `DonutFlyers`
- shepherd them through the goal
- finish the run when the gameplay song ends

The current problem is that the landscape ends, and the player can fly past that
edge and then travel underneath the landscape.

That means the map currently allows the player to leave the intended flight
space without a clean reject, recovery, or reset.

## Reviewed Context

Docs reviewed for this note:

- `README.md`
- `Docs/AGENT_BOOTSTRAP.md`
- `Docs/HANDOFF.md`
- `Docs/ACTIVE_TASK.md`
- `Docs/level1-media-playback.md`

Runtime files reviewed for this note:

- `Source/Zhoenus/SaveThemAllGameMode.cpp`
- `Source/Zhoenus/SpaceshipPawn.cpp`
- `Source/Zhoenus/DonutFlyerSpawner.cpp`

Map/config inspection used for this note:

- `Content/Map/Level-1.umap`
- `Config/DefaultEngine.ini`

## What We Verified

### 1. `Level-1` is the gameplay map, not lobby context

`Config/DefaultEngine.ini` routes `Level-*` maps back to
`/Game/Blueprints/SaveThemAllV1.SaveThemAllV1_C`, so `Level-1` is the live
`SaveThemAll` flight map rather than a menu shell.

### 2. The ship currently has no altitude clamp or out-of-bounds recovery path

`ASpaceshipPawn::Tick()` moves the ship with:

- `AddActorLocalOffset(LocalMove, true)`
- `AddActorLocalRotation(...)`

That means the ship is free 6DOF movement with sweep-based collision against
whatever colliders exist in the level.

No explicit player-containment path was found in runtime code for:

- `KillZ`
- world-bounds checks
- `FellOutOfWorld`
- altitude clamp / floor clamp
- teleport-back / reset when leaving intended flight space

So if the map exposes open space below or beyond the landscape, the pawn has no
second-line gameplay rule that pulls it back into the intended play volume.

### 3. Existing collision response is reactive, not containment-aware

`ASpaceshipPawn::NotifyHit()` deflects the ship when it collides with something,
but it only runs after a real collision occurs.

That is useful for geometry response, but it does not solve the case where the
player simply finds an open route past the landscape edge and into the
under-terrain space.

### 4. The project already treats the landscape like "ground" in one system

`ADonutFlyerSpawner` traces downward through the spawn box and rejects candidate
flyer spawns that are too close to the terrain surface.

That tells us two things:

- the map is already expected to have a meaningful ground/terrain surface
- the "stay above the landscape" rule exists for spawn validation, but only at
  spawn time

It is not currently a player-flight containment system.

### 5. The `Level-1` map asset does not show an obvious underside-containment actor

String inspection of `Content/Map/Level-1.umap` surfaced obvious actors such as:

- `Landscape_2`
- `Goal-v1_C_1`
- `DonutFlyerSpawner_1`
- `LightmassImportanceVolume_1`

The same inspection did not surface an obvious:

- `BlockingVolume`
- `PhysicsVolume`
- `TriggerVolume`

dedicated to stopping or recovering the player when leaving the intended
airspace.

This is not the same as a full editor inspection, but it is meaningful evidence
that the map currently does not advertise a clear containment volume through its
serialized actor names.

### 6. No obvious map-side `KillZ` or world-bounds strings were found

String inspection of `Content/Map/Level-1.umap` did not surface:

- `KillZ`
- `bEnableWorldBoundsChecks`

That is still weaker than opening the map in the editor and reading World
Settings directly, but it supports the current interpretation that there is no
clear map-authored fail-safe already waiting to catch under-landscape flight.

## Current Interpretation

This currently reads as a level-containment gap more than a ship-handling bug.

The ship is doing what the runtime allows:

- move freely in 6DOF
- stop only when collision exists
- continue into any open space the level exposes

So the failure mode is:

1. the landscape has a reachable edge / underside route
2. the ship can physically travel through that route
3. runtime has no fallback rule once the player is below the intended terrain

## Why This Matters

- It breaks the readability of `Level-1` as an intentional flyover playspace.
- It lets the player leave the core gather-and-save space without clear gameplay
  feedback.
- It risks awkward camera, reticle, and HUD composition against terrain
  undersides and void space.
- It weakens confidence in the level boundary in the same way that trap pockets
  weaken the Gate of Oblivion: the space stops reading as trustworthy.

## Containment Guardrail

The future `Level-1` boundary should either:

- reject the player
- deflect the player
- recover / reset the player

It should not silently admit the ship into under-landscape space.

## Open Questions Before Implementation

- Is the right fix map-authored containment, runtime recovery, or both?
- Is underside access limited to one seam, or is it available across a large
  perimeter of the landscape?
- Does this escape path affect only the player, or can `DonutFlyers` also enter
  invalid space around or below the terrain?
- Does the eventual solution need only collision containment, or does the
  landscape edge itself also need a readability / art treatment?
- Is there any intentionally valid off-landscape flight space that a hard
  invisible wall would block incorrectly?

## Recommended Next Validation

1. Open `Level-1` in the editor and inspect the exact route(s) that allow
   under-landscape flight.
2. Confirm the map's actual World Settings for any `KillZ` or world-bounds
   behavior.
3. Decide whether the preferred gameplay outcome is:
   - physical rejection
   - soft recovery
   - hard reset / fail state
4. Validate the chosen direction against the 6DOF save loop so containment does
   not make normal flight feel arbitrary or cramped.

## Scope Of This Pass

This pass documents the issue only.

No runtime behavior, map assets, collision setup, or player-movement code were
changed.
