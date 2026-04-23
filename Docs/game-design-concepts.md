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

Today `ZapEmProjectiles` are effectively infinite.

Current implementation facts:

- `ASpaceshipPawn::FireShot()` only checks:
  - `bCanFire`
  - `CurrentRateOfFire > 0`
- firing cadence is controlled by a timer, not a resource pool
- `AZapEmProjectile` is primarily a flyer-aggro tool:
  - it applies near-miss and hit threat to `DonutFlyers`
  - its effective aggro envelope is intentionally aligned with the visible reticle
- there is no current battery, light-harvest, or per-system energy budget in the
  live save/game-instance model

So the current mechanic is:

- hold fire
- spawn shots at the allowed cadence
- use those shots to influence follower behavior

That is the baseline reality, not the long-term concept.

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

- HUD battery readout
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
