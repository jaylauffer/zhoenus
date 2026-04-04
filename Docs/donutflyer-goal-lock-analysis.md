# DonutFlyer Goal-Lock Analysis

Date: 2026-03-31

## Issue

When a player flies through a `Goal`, follower DonutFlyers can lock onto that goal and then circle it indefinitely instead of crossing the trigger volume.

## Root Cause

- `AGoal::OnGoal` was locking followers to `sweepResult.ImpactPoint`, which is a one-time surface hit on the goal, not a stable approach target.
- `ADonutFlyerAIController::LOCKED` then drove the donut at that stale point with full forward thrust.
- The attempted circle-break path used `MoveRightInput(...)`, but `ADonutFlyerPawn` ignores that input in `LOCKED` and directly overwrites angular velocity from `TargetRot`.
- The `LOCKED` stuck check compared `Start == PreviousLocation`, which is too strict to be reliable for physics movement.
- Separately, the Gate of Oblivion mesh can still expose enterable trap pockets in its twisted windings. Even with a better lock target, those pockets create bad "orbit" or "stuck inside the gate" behavior because the geometry admits actors into spaces that are not part of the intended save path.

## Implemented Change

- Followers now lock to the goal's bounds origin instead of the overlap impact point.
- Locked steering now resolves the target from the live locked actor bounds each tick and aims through the target volume instead of at a single surface point.
- Locked thrust now scales by distance and can actively brake when the flyer is circling or very near the goal.
- Locked recovery now uses actual speed (`GetVelocity().SizeSquared()`) instead of exact position equality for the stuck transition.
- Lock acquisition resets the circling/progress tracking state so old rotation history does not leak into a new goal lock.

## Verification

- Code compiled successfully with `Build.bat ZhoenusEditor Win64 Development`.
- This pass did not include an in-editor gameplay run, so behavior against a live `Goal` still needs a short playtest.

## Next Check

- Run a gameplay test where a player drags one or more donuts through a stationary `Goal`.
- Confirm the donuts slow, steer through the goal volume, and stop orbiting outside the trigger.
- If they still overshoot, tune the locked distance/thrust thresholds in `ADonutFlyerAIController`.
- If they still get trapped or circle within the gate silhouette, treat that as a gate collision/layout problem rather than a pure controller-tuning problem. See `Docs/gate-clean-save-design.md`.
