# Reticle Principles

## Purpose

The reticle is not decoration.

In `Zhoenus`, the reticle is a forward flight instrument. Its job is to make
the ship's intent legible in 3D space so the player can fly and fire with
confidence instead of approximation.

The reticle should help the player answer three questions immediately:

- where is my shot actually going?
- is firing here likely to matter?
- how far ahead in space is the thing I am acting on?

If the reticle does not answer those questions well, the ship will feel vague
even if the underlying flight code is correct.

Range-finding is a core part of that job.

In practice, the reticle should help the player judge not only aim direction,
but whether a shot fired now has the reach and effective influence to matter
before the projectile expires.

## Why It Matters

`Zhoenus` is a 6DOF game with depth, closure, and angle management. That means
the player is not simply aiming at a flat screen position. They are trying to
understand a moving relationship between:

- ship orientation
- ship motion
- shot path
- world geometry
- `DonutFlyer` position and behavior

The reticle is the tool that makes that relationship readable.

When it works well, it should:

- make the ship feel precise
- connect steering and firing into one coherent action
- reduce the feeling of "guessing" in depth-heavy moments
- show the player when they have a meaningful line
- support the gather-and-save loop by clarifying when `ZapEmProjectiles` will
  likely influence a `DonutFlyer`

## Core Principles

### 1. The reticle is an instrument, not an ornament

The reticle must earn its place by conveying useful spatial information.

It should not exist only to look futuristic, pulse on fire, or sit at screen
center because that is what other games do.

### 2. The reticle should express intent, not just position

A center mark is not enough.

The reticle should communicate the live shot path and the space in which the
player's action is likely to matter. It should feel like an expression of the
ship's current capability, not a generic HUD sticker.

### 3. The reticle should reduce ambiguity in depth

The biggest challenge in 6DOF flight is not left versus right. It is depth,
distance, and alignment in forward space.

The reticle should improve the player's ability to judge:

- line
- range
- timing
- whether a firing opportunity is real or weak

### 4. The reticle should reward confidence, not encourage hesitation

The player should feel more able to commit, not more afraid to act.

Reticle feedback should say things like:

- this line is clean
- this shot has influence
- this target space is relevant

It should not drown the player in noisy status or make them second-guess every
input.

### 5. The reticle should align with gameplay truth

If the reticle implies that a shot should matter, gameplay should broadly agree.

That does not require pixel-perfect literalism, but it does require honest
alignment between:

- the visible reticle
- the shot path
- the projectile's effective influence space
- the player's practical expectation

This is especially important because `ZapEmProjectiles` influence flyers by more
than direct impact.

### 6. The reticle should stay world-anchored

The reticle should feel part of the same space the ship is flying through.

That means favoring a world-space, shot-path-oriented reticle over a flat screen
overlay unless there is a strong reason to compare approaches.

The player should feel like they are flying with an instrument in the world, not
dragging a cursor across the screen.

### 7. The reticle should support multiple layers of understanding

The reticle does not need to do everything with one visual move.

Useful layers include:

- baseline aiming reference
- range readability
- target relevance or presence
- confirmation or pulse feedback

These layers should build on each other rather than all arriving at once.

### 8. The reticle must remain readable across form factors

The reticle is higher priority than decorative HUD elements because it is
directly tied to moment-to-moment control.

It must survive:

- small screens
- handheld viewing distance
- touch play
- controller play
- desktop play

If a reticle idea only works on a desktop monitor, it is not yet finished.

## Player Value

At its best, the reticle should make the player feel:

- I understand where my ship is acting
- I know when I have a good line
- I can influence `DonutFlyers` on purpose
- my ship feels deliberate rather than vague

That is the real gameplay value. The reticle is there to strengthen confidence,
precision, and expressive control.

## Design Hierarchy

When judging reticle work, use this order:

1. readability
2. spatial truth
3. gameplay usefulness
4. responsiveness
5. visual style

If a change improves style but harms readability or trust, it is a regression.

## Anti-Patterns

The reticle should not drift into any of these:

- a purely cosmetic center-screen symbol
- a misleading promise that looks accurate but does not match gameplay
- a noisy status cluster trying to explain every system at once
- a platform-specific solution that only feels good on one screen class
- an implementation-driven feature that exists because the material supports it,
  not because the player needs it

## Practical Guidance

Future reticle work should begin by asking:

1. what player uncertainty does this change remove?
2. what useful action does this feedback support?
3. does the visible cue still match gameplay truth closely enough to be trusted?
4. is this change improving the reticle's role as a flight instrument, or just
   adding effect?

If those answers are weak, the change is probably not the right next reticle
step.

## Relationship To Runtime Notes

This document is about reticle intent.

For the current implementation, config, and runtime path, see:

- `Docs/reticle-and-flight-hud.md`
