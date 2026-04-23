# Zhoenus Game Design Concepts

Date: 2026-04-24

## Purpose

This note records higher-level game design concepts that are important to the
identity of `Zhoenus`, but are broader than the current implementation task list.

It exists to keep long-range ideas visible without pretending they are already
part of the shipping runtime.

## Core Loop Guardrail

`Zhoenus` is still centered on the `SaveThemAll` loop:

- fly the ship in 6DOF space
- gather `DonutFlyers`
- shepherd them through the goal
- earn end-of-run payout
- spend or convert durable progression on the power-up screens

Any projectile or energy mechanic should reinforce that loop, not turn the game
into a generic shooter or a constant resource-starvation sim.

## Current Runtime Truth

Today the live gameplay path has a first battery pass for `ZapEmProjectiles`.

Current implementation facts:

- `ASpaceshipPawn` owns a native `UZhoenusBatteryComponent`
- `ASpaceshipPawn::FireShot()` checks:
  - `bCanFire`
  - `CurrentRateOfFire > 0`
  - available battery energy for one shot
- firing cadence is still controlled by a timer
- a successful shot consumes battery energy
- the battery recharges over time
- the Canvas HUD now displays battery status
- `AZapEmProjectile` is primarily a flyer-aggro tool:
  - it applies near-miss and hit threat to `DonutFlyers`
  - its effective aggro envelope is intentionally aligned with the visible reticle
- there is still no light-harvest simulation
- no other ship systems currently consume battery energy

So the current mechanic is:

- hold fire
- spawn shots at the allowed cadence while battery charge lasts
- wait for passive recharge when the battery runs low
- use those shots to influence follower behavior

That is now the live phase-1 reality, not the full long-term concept.

## ZapEmProjectile Design Direction

The intended future direction is that `ZapEmProjectiles` should not be free.

They should consume an onboard ship resource, so firing becomes a meaningful
tactical choice instead of an unlimited background action.

Recommended gameplay reading:

- `ZapEmProjectiles` are not the main goal
- they are a control tool that helps gather and steer `DonutFlyers`
- they should therefore feel valuable and deliberate

## Working Mechanic Recommendation

The recommended practical mechanic is:

- the ship has an onboard battery / energy reserve
- each `ZapEmProjectile` costs a fixed amount of energy
- if there is not enough energy, the ship cannot fire
- battery energy replenishes over time

This is the smallest viable version of the larger idea.

Why this is the right first mechanic:

- it immediately makes shots non-infinite
- it is readable to the player
- it fits the existing fire-rate gate without requiring a total rewrite
- it does not require physically accurate light simulation on day one

## Projectile Radius And Battery

The expanding projectile radius should currently be read as shot effectiveness,
not as a second energy drain.

Current live relationship:

- battery / energy gates whether a shot can be fired at all
- once fired, the projectile's aggro radius expands over travel distance
- that expanding radius increases the shot's useful influence on nearby
  `DonutFlyers`

So the battery answers:

- how many shots the player can afford right now
- how quickly they recover from using those shots

The expanding aggro radius answers:

- how much space a single paid-for shot can influence
- how forgiving / expressive that shot feels as a control tool

Those are related in value, but not currently linked as separate costs.

This is the preferred phase-1 relationship:

- energy controls shot availability
- radius growth controls shot effectiveness

That keeps the system readable. A shot costs energy once, then delivers its
full designed gameplay effect.

If the project later wants deeper coupling, the safer direction is:

- upgrades that improve battery efficiency
- upgrades that improve projectile influence
- optional charged-shot behaviors

The less safe direction for feel is:

- charging extra battery continuously just because the aggro radius expands
- shrinking ordinary projectile usefulness whenever the battery is merely low

## Economies And Feel

`Zhoenus` already has more than one economy, and the battery now makes that
split explicit in runtime.

Current distinct economies:

- run-state ship performance:
  - live speed gains earned immediately during the `SaveThemAll` run
- durable progression currency:
  - `points` earned at run end
  - `points` reclaimed through `Convert`
  - `points` spent in `PowerUp` / `AdjustShip`

If `ZapEmProjectiles` are gated by `points`, those two economies collapse into one.

That is technically easy, but it creates the wrong feel:

- every trigger pull starts feeling like permanent upgrade loss
- the player is asked to burn long-term progression just to use a moment-to-moment
  control tool
- it encourages hoarding shots instead of using them to steer and gather
  `DonutFlyers`
- it makes in-run firing strength depend on how rich the player was before the run,
  instead of how well they are currently flying

That is why `points` are the wrong first firing gate even though they are the
cheapest implementation path.

The desired firing economy should instead feel:

- local to the current moment
- readable on the HUD
- recoverable after use
- tactical rather than permanently punishing

A rechargeable battery matches that feel much better than persistent `points`.

Design rule:

- durable progression currency should improve ship systems
- durable progression currency should not usually be deleted by ordinary trigger
  pulls

Good uses for `points`:

- buying better battery capacity
- buying better recharge rate
- buying better shot efficiency

Bad early use for `points`:

- paying directly for each `ZapEmProjectile`

## Larger Concept: Light Absorption

The older, more ambitious vision should remain a real design concept:

- the ship's material absorbs light
- absorbed light charges an onboard battery
- that battery powers multiple systems

Planned battery consumers:

- thrusters
- reticle range-finding
- `ZapEmProjectiles`

This concept is strong because it ties aesthetics, traversal, and ship systems
together. It gives the ship a coherent fiction instead of three unrelated bars.

## Staging Recommendation

The battery concept should be phased, not attempted all at once.

### Phase 1: Shot Cost Only

This phase is now the live baseline.

Add only:

- `CurrentEnergy`
- `MaxEnergy`
- `ZapShotEnergyCost`
- simple recharge over time

Do not make normal flight depend on the battery yet.

This phase answers the author-note problem directly:

- `ZapEmProjectiles` stop being infinite
- the core 6DOF loop remains intact

### Phase 2: Battery As A Visible Ship System

Add:

- power-up upgrades for:
  - battery capacity
  - recharge rate
  - shot efficiency

This makes the mechanic part of progression instead of a hidden limiter.

### Phase 3: Secondary System Power

Once the shot-cost version feels good, consider battery use for:

- reticle range-finding
- lock-assist or targeting helpers
- stabilization / auto-correct assistance

This is safer than making baseline thrust costly immediately.

### Phase 4: Light-Absorption Simulation

Only after the above works should the game consider a fuller concept where:

- light exposure meaningfully affects recharge
- ship materials and environment affect energy gain
- shade / brightness become part of spatial play

That is a richer concept, but it is not required for the first good version of
the mechanic.

## Important Guardrail: Do Not Tax Basic Flight First

If the first battery pass makes normal flight feel constrained or weak, it will
fight the identity of the game.

So the recommended order is:

1. make shots cost energy first
2. maybe make optional assist systems cost energy second
3. only later consider whether thrust itself should consume battery

`Zhoenus` is a flight game before it is a resource-management game.

## Power-Up Screen Relationship

This concept also fits the existing power-up flow cleanly.

Good future power-up candidates:

- max battery capacity
- battery recharge rate
- projectile energy efficiency
- reticle range-finding efficiency
- light absorption efficiency

Potentially bad early candidates:

- making the player buy back the right to perform ordinary flight
- turning every thrust input into a point-drain tax

The power-up flow should enhance ship expression, not punish movement basics.

## Recommended Working Decision

Until the full battery/light model exists, the intended design truth should be:

- `ZapEmProjectiles` are meant to consume battery energy
- the first implementation should use a simple rechargeable energy pool
- the broader "ship absorbs light and stores it in a battery" concept is the
  long-term fiction and future systems direction

That gives the project a concrete next mechanic and a larger conceptual north
star at the same time.
