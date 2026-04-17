# Reticle And Flight HUD

## Known Good State

This document describes the last known good reticle state after the reset.

The active aiming aid is a world-space reticle owned by `AZhoenusPawn`.
It is not the old HUD `+` overlay. The reticle is placed in the scene on the
projected shot path so the ship and level geometry can naturally clip it from
the camera view.

The current runtime path is:

- `ASpaceshipPawn::GetProjectileSpawnLocation()` returns the live muzzle origin
- `ASpaceshipPawn::GetProjectileFireDirection()` returns the live forward shot direction
- `ASpaceshipPawn::GetProjectileAimPoint()` line-traces on `ECC_Visibility` and returns either the hit point or the trace end
- `AZhoenusPawn::UpdateAimProjector()` places the reticle at that live aim point, clamps it by config distance, keeps a minimum forward standoff from the muzzle, and only then applies the depth-bias pullback
- `AZhoenusPawn::CreateAimProjector()` configures a `UMaterialBillboardComponent` using `/Game/Textures/M_AimProjector`

Relevant files:

- `Source/Zhoenus/SpaceshipPawn.cpp`
- `Source/Zhoenus/ZhoenusPawn.cpp`
- `Source/Zhoenus/ZhoenusPawn.h`
- `Config/DefaultGame.ini`

## Current Configuration

The known-good reticle settings currently live under `[/Script/Zhoenus.ZhoenusPawn]`
in `Config/DefaultGame.ini`.

Current values:

- `bEnableAimProjector=True`
- `AimProjectorMaterialPath=/Game/Textures/M_AimProjector.M_AimProjector`
- `AimProjectorTraceDistance=5000.0`
- `AimProjectorMaxVisibleDistance=3200.0`
- `AimProjectorMinVisibleDistance=160.0`
- `AimProjectorDepthBias=48.0`
- `AimProjectorScale=252.0`
- `AimProjectorAggroRadiusScale=0.5`

The legacy HUD reticle still exists in `ASpaceshipHUD`, but it is disabled by
config:

- `[/Script/Zhoenus.SpaceshipHUD]`
- `bDrawAimReticle=False`

That means the intended reticle path is the in-world billboard, not the screen overlay.

## Asset Path

The reticle art source is:

- `Assets/M_triangulation.png`

Imported Unreal assets:

- `/Game/Textures/M_triangulation`
- `/Game/Textures/M_AimProjector`

The triangle art should remain a clean inverted translucent lime-green marker.
The asset should read clearly at gameplay distance without becoming a large
opaque panel.

## Reticle And Aggro Radius

There is an important gameplay design goal behind the reticle:

- if a `DonutFlyer` is visually inside the reticle, a fired `AZapEmProjectile`
  should be able to aggro it as the projectile passes through that reticle space

Current code paths now have an explicit shared handoff.

What the code does now:

- the reticle billboard is created with `AddElement(..., false, AimProjectorScale, AimProjectorScale, ...)`
- because `bSizeIsInScreenSpace` is `false`, the billboard size is treated as world size
- the current reticle quad is therefore approximately `252 x 252` world units
- `AZhoenusPawn::GetProjectileAggroRadius()` derives the gameplay radius from the visual reticle using `AimProjectorScale * AimProjectorAggroRadiusScale`
- with the current values, the projectile aggro radius is `126` world units
- `ASpaceshipPawn::FireShot()` passes that radius into each spawned `AZapEmProjectile`
- `AZapEmProjectile` applies that override to `AggroSphere`, falling back to `BaseProjectileRadius * AggroRadiusMultiplier` only when no override is provided

So today:

- reticle size is driven by `AimProjectorScale`
- projectile aggro size for shots fired by `AZhoenusPawn` is driven from that same reticle size
- the `ProjectileAggroRadiusMultiplier` path remains as a fallback for any projectile that is spawned without a reticle-owning attacker

That means the visual reticle and the actual projectile near-miss envelope now stay aligned for the player ship.

## Remaining Design Direction

The current implementation is intentionally narrow:

1. `AZhoenusPawn` owns the visible reticle
2. `AZhoenusPawn` exposes the effective gameplay radius for that reticle
3. fired projectiles inherit that exact radius

That is enough to keep the player's visible reticle and projectile aggro envelope in sync.

What still remains open:

- whether ship customization should continue to express this as `ProjectileAggroRadiusMultiplier`
- whether the project should eventually replace that multiplier with a direct shared radius in `FShipStats`
- whether different ships should expose different visual-to-gameplay scaling through their own overrides

## Design Goals

The reticle should serve the game across phone, PC, Mac, consoles, and Switch.
That means the design goal is not just "looks good on a desktop monitor." It must:

- stay readable on smaller screens and at handheld viewing distance
- work with controller, keyboard/mouse, and touch-friendly play styles
- feel anchored in the world instead of pasted on top of the scene
- show where `AZapEmProjectile` is expected to travel
- allow the ship and nearby geometry to clip it naturally from the camera view
- remain visually simple enough to survive different resolutions and post-process settings

## What Counts As Correct

The reticle is correct when all of these are true:

- it is visible in front of the ship during normal flight
- it sits on the live shot path, not an arbitrary camera ray
- it does not overlap the ship like a HUD sticker
- it hides instead of collapsing back onto the muzzle when the trace is too close to satisfy the forward standoff
- the ship can partially or fully occlude it
- it remains legible without dominating the frame
- it behaves consistently while flying and firing

## Current HUD State

The HUD still draws flight bars in `Source/Zhoenus/SpaceshipHUD.cpp`.
The pitch feedback there is still a placeholder bar driven directly by
`CachedInput.X`. It is not yet a polished aircraft-style instrument.

Future HUD direction should favor:

- a centered signed pitch ladder or horizon reference
- symmetric up/down pitch feedback
- attitude-oriented feedback rather than raw input-only feedback
- presentation that remains readable on smaller displays

## Guardrails For Future Reticle Work

The reticle recently drifted because multiple concerns were changed at once:
renderer choice, material graph, color pulse behavior, projectile timing, and
asset presentation. Do not repeat that.

When iterating on the reticle:

- keep one known-good visible baseline at all times
- change one layer at a time
- first prove plain visibility, then add polish
- do not change renderer path and material behavior in the same pass
- do not add firing feedback until the plain green reticle is stable
- prefer shared projectile helpers in `ASpaceshipPawn` over duplicate aim math
- keep the HUD overlay disabled unless intentionally comparing approaches

Safe order of operations:

1. verify visibility and placement
2. tune scale and distance
3. tune art and material
4. add feedback such as pulse, color shift, or hit confirmation

## Non-Goals For The Baseline

The known-good baseline does not need to solve everything at once.
It does not yet need:

- firing pulse behavior
- dynamic color shift
- projectile-synchronized animation
- a redesigned pitch instrument
- a new renderer path unless the baseline stops being reliable

The baseline job is simpler: provide a stable in-world aiming reference that can
be tested and built on safely.
