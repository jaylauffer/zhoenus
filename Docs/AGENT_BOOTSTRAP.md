# Zhoenus Agent Bootstrap

Read this before making changes.

## Project Identity

`Zhoenus` is a UE5 6DOF spaceship flying-action game.

The core loop is `SaveThemAll`:

- fly the ship in full 6DOF space
- round up `DonutFlyers`
- shepherd them through the goal
- earn end-of-run points
- spend or convert ship capability on the power-up screens

Do not drift into generic UE assumptions. This project is about spaceship handling, follower behavior, goal saving, and the point/power-up loop.

## Required Reading Order

Before proposing changes, read:

1. `README.md`
2. the most relevant file in `Docs/`
3. the directly affected runtime source files in `Source/Zhoenus/`

If the task touches saving, scoring, followers, goals, ship handling, or power-up flow, say which docs and source files you reviewed before editing.

If the task touches `Level-1` presentation or in-world video playback, also read:

- `Docs/level1-media-playback.md`
- `Source/Zhoenus/LevelVideoSurfaceManager.cpp`
- `Source/Zhoenus/SaveThemAllGameMode.cpp`

## Key Runtime Files

Primary gameplay loop:

- `Source/Zhoenus/SaveThemAllGameMode.cpp`
- `Source/Zhoenus/SaveThemAllGameInstance.cpp`
- `Source/Zhoenus/SaveThemAllGameState.cpp`
- `Source/Zhoenus/SaveThemAllPlayerController.cpp`
- `Source/Zhoenus/SpaceshipPawn.cpp`
- `Source/Zhoenus/Goal.cpp`
- `Source/Zhoenus/DonutFlyerPawn.cpp`
- `Source/Zhoenus/DonutFlyerAIController.cpp`
- `Source/Zhoenus/DonutFlyerSpawner.cpp`

Power-up and point flow:

- `Source/Zhoenus/AdjustShipUI.cpp`
- `Source/Zhoenus/ConvertSpeedUI.cpp`
- `Source/Zhoenus/PowerUpRootUI.cpp`
- `Source/Zhoenus/PowerUpScreenUI.cpp`
- `Source/Zhoenus/PowerUpStatWidgetUI.cpp`
- `Source/Zhoenus/ShipStats.cpp`

## Current Gameplay Truths

- The ship is a 6DOF flying craft, not a character controller.
- `DonutFlyers` are followers to be gathered and saved through the goal.
- The end-of-run payout matters.
- The power-up screens are part of the gameplay loop, not an afterthought.

## Current Design Priorities

These are active concerns reflected in the repo docs:

- `Docs/donutflyer-goal-lock-analysis.md`
  Followers should lock and pass through the goal cleanly instead of orbiting.
- `Docs/gate-clean-save-design.md`
  The Gate of Oblivion must either reject or save, never trap.
- `Docs/point-system-and-convert.md`
  Points and convert flow need to remain coherent and player-readable.
- `Docs/gamepad-controls-analysis.md`
  Control feel and navigation matter.

## Agent Working Rules

- Restate the relevant gameplay loop before editing.
- Prefer minimal, direct fixes over speculative refactors.
- Keep focus on making the game better now, not adding infrastructure unless it clearly supports the loop.
- If a task touches UI, input, ship feel, goal saving, or the power-up flow, explain the expected player-facing behavior change.
- Do not ignore existing docs in `Docs/`.

## Expected Opening Behavior

At the start of a task:

- identify the relevant docs you read
- identify the relevant `Source/Zhoenus/*.cpp` and `.h` files
- summarize the gameplay behavior under discussion in 2-4 lines

If you cannot do that, you are not ready to edit the repo.
