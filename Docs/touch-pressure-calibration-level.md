# Touch Pressure Calibration Level

Date: 2026-04-22

## Purpose

`TouchConfigPressure` is a standalone calibration map for touch-pressure tuning.

It does not modify `Level-1`, `PowerUp`, `Startup`, or the live `SaveThemAll`
gameplay path. Its only job is to let a player tune touch-pressure values for:

- `StabilizePressureDeadzone`
- `FirePressureDeadzone`
- `StabilizePressureScale`
- `FirePressureScale`

## Runtime Routing

- Map asset path: `/Game/Map/TouchConfigPressure`
- Map prefix: `TouchConfig`
- Routed game mode: `/Script/Zhoenus.TouchPressureCalibrationGameMode`
- Quick launch: `open TouchConfigPressure`

The prefix route keeps the calibration experience isolated from the existing map
set. No older map needed world-settings edits.

## UI Behavior

The calibration screen is native C++ UI, not a blueprint widget. It renders a
gamepad-style layout with:

- a left thumbstick preview for `Stabilize`
- a right thumbstick preview for `Fire`
- live raw touch-force readouts
- live normalized output bars after deadzone + scale
- save/reset controls
- touch and mouse release events preserve the Save and Reset buttons instead of
  swallowing every pointer-up at the root widget

Mouse is supported as a fixed-force preview path for desktop iteration. Real
touch-force preview still depends on the platform providing `GetTouchForce()`.

## Persistence

Values are stored in `UZhoenusTouchPressureSettings` under the game config
section:

- `/Script/Zhoenus.ZhoenusTouchPressureSettings`

The calibration level can save updated values without changing the existing
flight code. A later gameplay pass can read from the same settings object when
the team is ready to wire these values into the live touch path.
