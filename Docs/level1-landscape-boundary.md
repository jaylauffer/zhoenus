# Level-1 Planet Boundary Note

Date: 2026-04-26

## Purpose

Document the current `Level-1` outer-world model in a way that stays simple and
playable across console/desktop, tablet, `iOS`, and `Android`.

## Gameplay Behavior Under Discussion

`Level-1` is the live `SaveThemAll` map:

- fly the ship in readable 6DOF space
- gather `DonutFlyers`
- shepherd them through the goal
- finish the run when the gameplay song ends

The original boundary failure was that the player could leave the authored
landscape and travel underneath the map.

## What Changed

The project is no longer treating this as “keep growing the baked landscape.”

It is also no longer treating the procedural outer surface as the authority.

The current model is:

1. `APlanetBody` is the authority.
2. `ASpaceshipPawn` enforces the authority.
3. `APlanetSurfaceRuntime` is optional visual continuation.

That is the smallest model that still respects the real problem:

- visuals can lag
- physics rules cannot

## Simple Planet Concept

### `APlanetBody`

Owns the world-law data:

- `PlanetCenter`
- `PlanetRadius`
- `GuardrailClearance`
- authored core bounds built from relevant non-sky level actors

It initializes itself from the current gameplay ground and then exposes simple
sphere queries.

### `ASpaceshipPawn`

Does not depend on one specific visual runtime actor anymore.

It now selects the nearest valid `APlanetBody` and applies one cheap rule:

- if the ship is moving into the forbidden inside-shell region, push it back to
  `surface + clearance`
- remove the inward component of motion
- keep the rest of the 6DOF feel intact

This is the actual containment rule.

Important constraint:

- authored `Level-1` core space wins over the planet barrier
- the guardrail only applies outside that authored core
- the goal route and any other intentional static gameplay geometry inside the
  authored core must remain playable even if they sit below the notional
  spherical shell

### `APlanetSurfaceRuntime`

Uses the active `APlanetBody` for planet math and only draws the cheap outer
surface continuation.

That means:

- if runtime generation is slow, visuals may pop
- but the player should still respect the planet boundary

## Why This Is Better

- It is cheaper than making collision depend on generated terrain.
- It is more reliable than trying to outrun a visual system.
- It is simple enough to carry across desktop, console, tablet, `iOS`, and
  `Android`.
- It can scale to future multiple planets because the ship queries a planet
  body, not a special-case `Level-1` landscape hack.

## Current Runtime State

`ASaveThemAllGameMode` now spawns:

- `APlanetBody` when planet boundary is enabled
- `APlanetSurfaceRuntime` when planet surface visuals are enabled

Current defaults:

### Planet authority
- `PlanetRadius=1500000.0`
- `GuardrailClearance=350.0`
- `AuthoredCoreBoundsPadding=12000.0`

### Planet visuals
- `TileWorldSize=30000.0`
- `TileResolution=8`
- `TileRingCount=2`
- `RebuildDistance=12000.0`

## Validation Status

Verified:

- `ZhoenusEditor` builds with the split model.
- Headless `Level-1` boot shows:
  - planet body authored-core bounds cache
  - planet body initialization
  - planet surface runtime waiting outside the authored core

Not yet proven:

- final gameplay feel at the handoff
- whether the current radius / clearance / tile values feel right in live play

## Design Guardrails

- Keep `APlanetBody` as the authority.
- Keep `ASpaceshipPawn` responsible for enforcement.
- Keep `APlanetSurfaceRuntime` visual-only.
- Do not re-couple containment to mesh generation.
- Do not fall back to blocker walls or blind static landscape growth.

## Future Multi-Planet Reality

Do not over-complicate this yet.

The current intended path is:

- each planet gets an `APlanetBody`
- the ship selects the nearest valid body
- each body may optionally have a surface runtime companion

That is enough for now.

If future gameplay needs more than nearest-body selection, solve that only when
the game actually reaches that point.
