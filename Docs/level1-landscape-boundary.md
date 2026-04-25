# Level-1 Landscape Boundary Note

Date: 2026-04-25

## Purpose

Document the current `Level-1` landscape-edge problem and the agreed first
implementation direction.

This note started as a pure diagnosis pass. It now also records the user-facing
constraints that came out of follow-up discussion so later work does not drift
back toward blocker walls, runtime recovery rules, or oversized terrain scope.

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
space by reaching a terrain seam and then traveling underneath the landscape.

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

## Discussion Outcome

Follow-up discussion narrowed the boundary expectations for `Level-1`:

- the whole `Level-1` map is intended to be playable space
- players should remain effectively unbounded in the air
- perimeter blocker rings are a poor fit because they change the flight feel
- mobile support matters, so simply covering a huge "planet" with expensive
  landscape is also a poor fit

The current chosen direction is:

- extend the reachable terrain outward with flat, low-cost landscape
- keep that extension visually and technically cheap compared to the authored
  core of the map
- use that outer band to move the hard terrain edge farther away from normal
  play rather than boxing the player in with side blockers

This is intentionally different from:

- a hard perimeter `BlockingVolume` solution
- a terrain-aware runtime recovery rule based on "below terrain at this XY"
- a full high-detail landscape expansion around the entire map

## Design Guardrails

The current doc set should assume the following unless the task changes again:

- the first iteration is a map-side landscape expansion task, not a pawn
  movement rewrite
- the added terrain should be mostly flat and low cost
- the extension should cover the reachable seam, not try to build a literal
  full planet
- the fix should preserve the open 6DOF feel instead of creating side-wall
  behavior
- mobile targets matter, so the outer band should stay cheaper than the
  authored gameplay core

## Open Implementation Questions

- Which edge or edges of the current landscape are actually reachable in normal
  play and should be extended first?
- How wide does the first flat outer band need to be to make the current seam
  effectively unreachable during a normal run?
- Can the outer band reuse the current landscape material as-is, or does it
  need a simpler treatment for mobile cost?
- Which visual or gameplay details should remain confined to the authored core
  so the extension stays cheap?

## Recommended Next Work

1. Open `Level-1` in the editor and inspect the exact reachable seam route or
   routes.
2. Inspect the current landscape layout and determine the smallest practical
   flat extension pattern.
3. Add a conservative first outer band around the reachable edge instead of a
   full all-direction expansion.
4. Validate that normal 6DOF play, flyer gathering, goal saving, and the
   song-ended transition still behave the same after the map change.

## Current Implementation Direction

The first iteration should aim for a practical mobile-safe terrain extension:

- add only the extra landscape needed to push the reachable seam away from
  normal runs
- keep the outer terrain mostly flat and cheap
- avoid adding dense sculpt detail, gameplay clutter, or heavy material cost in
  that outer band
- treat the existing authored play zone as the high-value gameplay core

Because the project targets `iOS` and `Android`, this extension should stay
conservative. The goal is not to make an enormous world. The goal is to make
the current landscape edge effectively unreachable during normal play without
damaging the open 6DOF feel.

## Scope Of This Pass

This pass documents the issue and the agreed direction only.

No runtime behavior, map assets, collision setup, or player-movement code were
changed.
