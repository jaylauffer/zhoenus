# HUD Current State

## Scope

This document describes the present HUD implementation in the reset baseline.
It is a snapshot of what the game is actually drawing now, not a proposal for
what the HUD should become later.

Relevant code paths:

- `Source/Zhoenus/SaveThemAllGameMode.cpp`
- `Source/Zhoenus/SpaceshipHUD.h`
- `Source/Zhoenus/SpaceshipHUD.cpp`
- `Source/Zhoenus/SpaceshipPawn.h`
- `Source/Zhoenus/SpaceshipPawn.cpp`
- `Config/DefaultGame.ini`

## Ownership And Activation

`ASaveThemAllGameMode` sets `HUDClass = ASpaceshipHUD::StaticClass()`, so the
active gameplay HUD is the custom `ASpaceshipHUD`.

At startup, `ASpaceshipHUD::BeginPlay()` also instantiates the UMG widget class
at `/Game/Blueprints/HUD` and adds it to the viewport. The C++ HUD code does
not currently drive that widget further. The code-owned HUD behavior is in
`DrawHUD()`.

## Current Draw Stack

The active HUD has two layers:

- a UMG widget loaded from `/Game/Blueprints/HUD`
- immediate-mode Canvas drawing in `ASpaceshipHUD::DrawHUD()`

The Canvas path is the part we can describe precisely from source.

## Canvas Elements

`DrawHUD()` only draws its instruments if the owning pawn is an `ASpaceshipPawn`.

Current Canvas elements:

- optional screen-space aim triangle
- forward and reverse speed bar
- thrust / acceleration bar
- pitch indicator triangles
- roll bars
- yaw indicator triangles
- stabilize bar

There are no text labels, tick marks, units, or calibration markers in this
Canvas layer.

## Reticle State

The old HUD reticle path still exists:

- `DrawAimReticle()` projects `Pawn.GetProjectileAimPoint(AimReticleDistance)` to screen space
- `DrawAimTriangle()` renders a lime-green outline triangle with `DrawLine()`

However, the HUD reticle is currently disabled in `Config/DefaultGame.ini`:

- `[/Script/Zhoenus.SpaceshipHUD]`
- `bDrawAimReticle=False`

The intended aiming path right now is the world-space reticle owned by
`AZhoenusPawn`, not this overlay triangle.

## Bar Layout

The current layout is hard-coded in normalized screen-space percentages.

Right-edge cluster:

- speed at about `98%` width, `70%` height
- thrust at about `97%` width, `70%` height
- stabilize at about `97%` width, `70%` height with a further left offset

Roll:

- left roll bar at about `4%` width, `30%` height
- right roll bar at about `96%` width, `30%` height

Yaw:

- vertically centered near `50%` height
- left triangle anchored about `16%` in from the left edge
- right triangle anchored about `16%` in from the right edge

Pitch:

- centered horizontally
- top triangle base anchored near `12%` screen height
- bottom triangle base anchored near `88%` screen height

This means the current HUD is a hand-placed set of bars, not a single coherent
instrument panel layout.

## Data Sources

The bars read directly from pawn state:

- speed uses `CurrentForwardSpeed`
- thrust uses `CachedInput.W`
- pitch uses `CachedInput.X`
- yaw uses `CachedInput.Y`
- roll uses `CachedInput.Z`
- stabilize uses `StabilityInput.X`

This is important: most of the HUD reflects current input state, not actual ship
attitude or world-relative flight state.

## Current Visual Behavior

### Speed

- forward speed is red
- reverse speed is yellow
- the bar only appears when speed magnitude exceeds about `4.6`
- forward and reverse scales are different:
  - forward uses `Canvas->ClipY * 0.6`
  - reverse uses `Canvas->ClipY * 0.2`

### Thrust

- positive thrust is blue
- negative thrust is green
- positive and negative use different vertical scales:
  - positive uses `0.6`
  - negative uses `0.2`

### Pitch

- pitch uses two centered translucent filled triangles instead of the old right-edge bar
- it reads `-CachedInput.X`
- the top triangle represents positive pitch input
- the bottom triangle represents negative pitch input
- active direction uses tip travel plus opacity as feedback
- both triangles keep a very faint baseline opacity so the anchors remain readable
- the active direction keeps its baseline fixed and pushes the tip farther outward as input magnitude approaches `1`
- the active direction also strengthens toward the configured orange max alpha as input magnitude approaches `1`

This is a clearer directional cue than the old magenta bar, but it is still an
input display, not a true pitch instrument.

### Roll

- roll is violet / blue-purple
- positive roll draws on the left side of the screen
- negative roll draws on the right side of the screen
- both directions use the same `0.6` scale

### Yaw

- yaw uses two translucent green filled triangles instead of the old top-center bar
- it reads `-CachedInput.Y`
- the left triangle represents positive yaw input
- the right triangle represents negative yaw input
- both triangles use the same translucency envelope as the pitch indicators
- the active direction keeps its base fixed and pushes the tip farther outward as input magnitude approaches `1`

This is a clearer left/right cue than the previous wide bar, but it is still an
input display, not a true flight reference.

### Stabilize

- stabilize is purple
- it reads `-StabilityInput.X`
- like pitch and thrust, it uses asymmetric positive / negative scales:
  - positive uses `0.6`
  - negative uses `0.2`

## Present Limitations

The current HUD works as a raw debug-style flight-state display, but it has
clear limitations:

- most bars visualize input, not aircraft attitude or target state
- several channels use asymmetric positive vs negative scaling
- there are no labels, legends, or units
- the layout is hand-tuned by screen percentage rather than systemized
- the reticle overlay path still exists in code even though world-space reticle is preferred
- pitch feedback is still input-driven rather than attitude-driven
- yaw feedback is still input-driven rather than attitude-driven
- there is no documented mobile-specific HUD adaptation yet

## Pitch State

Pitch is now in a better place visually, but it is still an early pass.

What it is now:

- a centered top/bottom orange triangle pair
- rendered as translucent Canvas triangles rather than line-drawn outlines
- driven directly by `CachedInput.X`
- centered as a signed directional cue
- not based on actual ship attitude
- more legible than the previous right-edge bar on small displays
- still not visually integrated with a full flight-instrument language

The switch to filled Canvas triangles matters because Unreal's `FCanvasLineItem`
is effectively opaque-only, which made the earlier line-drawn pitch triangles
look far more solid than their alpha values suggested.

So the present pitch display is closer to "current input pressure" than
"aircraft pitch instrument."

## Platform Implications

If Zhoenus is meant to work on phone, PC, Mac, console, and Switch, the current
HUD should be treated as a functional prototype, not a final interface.

The current HUD does prove:

- the game has a working custom `AHUD`
- the ship exposes enough state to drive instrumentation
- the Canvas layer can show thrust / pitch / yaw / roll / stabilize information

It does not yet prove:

- readability on small screens
- touch-friendly clarity
- a polished flight-instrument language
- cross-platform consistency

## Baseline Guidance

For future work, this present state should be preserved as a reference point.

That means:

- keep this document aligned with the live HUD code
- treat the current Canvas bars as baseline instrumentation, not sacred design
- improve one channel at a time, with pitch now moved to the new triangle cue baseline
- avoid combining HUD redesign, reticle redesign, and control remapping in the same pass

The next sensible HUD-specific task is to evolve pitch from an input cue into an
attitude-oriented instrument without changing the rest of the flight
instrumentation at the same time.
