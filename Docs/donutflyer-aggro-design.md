# DonutFlyer Aggro Design

Date: 2026-03-31

## Purpose

This note documents the current DonutFlyer aggro behavior, the inconsistencies in the existing implementation, and the recommended design going forward.

## Current Behavior

### 1. Each DonutFlyer controller has its own aggro map

`ADonutFlyerAIController` owns a per-donut `ThreatMap` keyed by player pawn.

Current local channels:

- `ProjectileThreat`
- `SightThreat`
- `CollisionThreat`

`GetTargetPlayer()` chooses the player with the highest total from this local map.

### 2. Proximity and line of sight add local `SightThreat`

In `SEARCHING`, the controller:

- overlaps nearby pawns inside `SearchDistance`
- line traces to visible `ASpaceshipPawn` targets
- adds a sight pulse through `ApplyAggroEvent(...)`

This means sight is currently both detection and target scoring.

### 3. Projectiles add local `ProjectileThreat`

`AZapEmProjectile` applies aggro directly to the specific donut it overlaps or hits:

- near pass through `AggroSphere`: `ProjectileNearMissThreat`
- direct hit: `ProjectileHitThreat`

That path now talks to the specific donut's controller through `ApplyAggroEvent(...)`.

### 4. PlayerState also stores separate "aggro" values

`AZhoenusPlayerState` has its own:

- `DonutCollisionScore`
- `DonutShotScore`
- `DonutSightScore`

Those values are modified by gameplay events too:

- `DonutFlyerPawn::NotifyHit` records collision telemetry
- `ZapEmProjectile` records shot telemetry on direct hit
- `ZhoenusSoccerGameMode::ClearAfterGoal` logs and clears them

### 5. The two aggro stores are not the same system

Important difference:

- Donut target selection reads `ADonutFlyerAIController::ThreatMap`
- PlayerState telemetry values are bookkeeping and logging

Today, those systems overlap in naming but not in authority.

### 6. Collision aggro now writes to both controller state and telemetry

`DonutFlyerPawn::NotifyHit` now does two separate things:

- applies `CollisionBump` to the specific donut controller through `ApplyAggroEvent(...)`
- records the event on `AZhoenusPlayerState` for telemetry

That is the intended split: controller for AI behavior, PlayerState for stats.

### 7. Aggro decay is mostly controller-local now

The controller-local map decays every tick:

- `CollisionThreat`: `CollisionThreatDecayPerSecond`
- `ProjectileThreat`: `ProjectileThreatDecayPerSecond`
- `SightThreat`: `SightThreatDecayPerSecond`

There is also extra decay in the controller's `STUCK` state, and that now reduces the controller-local threat map instead of `AZhoenusPlayerState`.

### 8. Goal lock is not aggro

`LockTarget()` moves the donut into `LOCKED` with a non-player target such as `AGoal`.

This is objective behavior, not threat scoring.

## Design Problem

The current code mixes three different concerns:

- sensory evidence: "I can see this player"
- hostility/threat: "this player shot or rammed me"
- objective commitment: "I am currently locked to a goal"

Those are related, but they should not be represented by partially duplicated variables in different classes.

## Recommended Design

### 1. Make the DonutFlyer controller the authority for aggro

The per-donut controller should be the only gameplay-authoritative source for player aggro.

That means:

- all events that change who a donut wants to chase should flow through one API on `ADonutFlyerAIController`
- target selection should read only that controller-owned data
- decay and weighting should live in that same system

### 2. Treat PlayerState values as telemetry, not AI state

`AZhoenusPlayerState` can still keep player-facing counters, but those should be treated as stats, not as the source of truth for AI behavior.

Preferred direction:

- rename the PlayerState fields so they read like stats instead of aggro
- or remove them if they are no longer needed for scoring/UI/debugging

### 3. Separate aggro from objective locks

Aggro should answer:

- which player does this donut prefer to chase right now?

Objective lock should answer:

- is this donut temporarily committed to a non-player objective?

`LOCKED` goal behavior should remain separate from aggro scoring. Aggro can still accumulate in the background, but it should not be confused with the goal-lock target.

### 4. Normalize all aggro sources into explicit event types

Recommended aggro events:

- `SightPulse`
- `ProjectileNearMiss`
- `ProjectileHit`
- `CollisionBump`
- `ScriptedThreat` if a designer wants a direct aggro push

Each event should provide:

- source player pawn
- magnitude
- event type
- whether it should wake the donut from idle/searching

### 5. Separate global Donut AI tuning from player ship customization

Current tuning is split across:

- Donut AI threat weights and decay rates
- Donut sight/detection behavior
- the projectile aggro radius that belongs to the firing player's ship

Recommended direction:

- keep global Donut AI behavior in one Donut aggro tuning struct or data asset
- keep player-owned projectile aggro radius on `FShipStats`

Reason:

- projectile near-miss/hit threat values are game balance and should affect all players consistently
- projectile aggro radius changes how a specific player's ship attracts donuts and fits the spend-points customization path

## Practical Rules

The design should follow these rules:

- Only events that happened to or near a specific donut should change that donut's aggro.
- A projectile near-miss should affect only the donuts the projectile actually passed near.
- A collision bump should affect the donut involved in that collision.
- Sight should be a low recurring pulse while a player remains visible, not a separate competing system.
- Goal locking should override movement intent, not overwrite the meaning of aggro.

## Suggested API Shape

One reasonable direction is a controller entry point like:

`ApplyAggroEvent(APawn* Source, EDonutAggroEventType Type, float Magnitude, bool bWakeDonut = true)`

That method would:

- validate the source
- update the correct local aggro channel
- handle state wake-up rules
- keep all aggro bookkeeping in one place

## Recommended Refactor Order

1. Decide that controller-local aggro is the gameplay authority.
2. Convert projectile, sight, and collision flows to a single controller aggro API.
3. Stop using `AZhoenusPlayerState` aggro fields as if they were AI state.
4. Rename PlayerState fields or replace them with explicit stats/debug counters.
5. Move aggro weights and radii into one tuning struct or data asset.
6. Remove dead or misleading logic, especially any decay that touches PlayerState instead of controller aggro.

## Implementation Status

Implemented in this pass:

- `ADonutFlyerAIController` now exposes a single `ApplyAggroEvent(...)` entry point for aggro writes.
- Sight pulses, projectile near-miss, projectile hit, and collision bump now all feed the controller through that single API.
- The controller `STUCK` fallback now decays controller-local aggro instead of trying to decay `AZhoenusPlayerState`.
- `AZhoenusPlayerState` now uses telemetry-oriented names instead of AI-facing aggro names.
- Donut AI weights/decays now live in `FDonutAggroTuning`.
- `ProjectileAggroRadiusMultiplier` intentionally remains under `FShipStats` because it belongs to player ship customization.

## Reticle Alignment Goal

There is a related ship-facing design goal for projectile aggro:

- a `DonutFlyer` that is visually inside the player's reticle should be aggroed
  by a fired projectile passing through that same space

Current code path:

- `AZhoenusPawn` visualizes the shot path with a world-space reticle billboard
- `AZapEmProjectile` aggroes through `AggroSphere`
- `AZhoenusPawn::GetProjectileAggroRadius()` now derives an effective gameplay radius from the reticle scale
- `ASpaceshipPawn::FireShot()` passes that radius into each spawned `AZapEmProjectile`
- `AZapEmProjectile` uses that override for `AggroSphere`, falling back to `BaseProjectileRadius * ProjectileAggroRadiusMultiplier` only when no override is supplied

That means player-fired shots from the Zhoenus ship now keep their visual reticle and projectile near-miss envelope aligned.

If the design goal matters, the better architecture is:

- keep the aggro authority in `ADonutFlyerAIController`
- keep projectile near-miss as a projectile-driven gameplay event
- keep sourcing both reticle size and projectile aggro radius from one shared ship-facing value

That would preserve the current aggro architecture while making the player's
visual aiming aid and the projectile's effective near-miss envelope describe the
same gameplay space.

Not yet implemented:

- Converting any future scripted aggro events to the same controller API.
- Moving Donut aggro tuning out of the game instance/save structs if the project later wants a dedicated data asset workflow.

## Bottom Line

The right approach is not "projectile aggro versus proximity aggro." Both are valid aggro inputs.

The real design boundary should be:

- DonutFlyer controller owns aggro
- gameplay events feed that controller through one API
- PlayerState stores stats only
- goal locks stay separate from aggro
