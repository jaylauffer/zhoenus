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
- live raw, travel fallback, and effective pressure readouts
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

The live touch path now reads those settings for `Stabilize` and `Fire`.
When the platform reports a real force value, Zhoenus uses it. Unreal's
ordinary-touch default force of `1.0` is treated as a full press to preserve the
pre-calibration fire cadence. When Unreal reports `0.0`, Zhoenus falls back to
thumbstick travel so pressure-mode controls remain viable on devices without a
usable force signal.
