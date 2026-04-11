# Zhoenus Input and Gamepad Analysis

Date: 2026-03-31

## Scope

This note documents the current state of the Zhoenus player-input stack, with emphasis on the recent gamepad-control fixes and the remaining follow-up work.

## Executive Summary

- The active flight controls now run through Enhanced Input on `ASpaceshipPawn`, using `Content/Input/DefaultMappingContext.uasset` and the action assets under `Content/Input/Actions/`.
- The most recent gamepad fix was commit `4fb60f7` on 2026-03-31. It changed the ship input handlers to use analog `Axis1D` values consistently, added `Completed` and `Canceled` bindings so axes clear correctly, and moved firing into the pawn's Enhanced Input path.
- The active ship blueprint `Content/Blueprints/Ships/ZhoenusSpaceShip.uasset` references `DefaultMappingContext`, not `LoadngoInputMapping`. `Content/Input/LoadngoInputMapping.uasset` looks orphaned.
- The project still uses a hybrid input setup: Enhanced Input for flight, touch buttons through `AZhoenusPlayerController`, legacy axis mappings in `Config/DefaultInput.ini`, and a custom RawInput device profile.
- `FireAction` was verified in-editor on 2026-03-31. The live ship mapping binds fire only to `Gamepad_RightTriggerAxis`; there is no mouse-fire binding in the active `DefaultMappingContext`.
- The next step should not be another blind mapping tweak. The next step should be to consolidate the active input path, verify device detection against real runtime data, and run a short platform/controller test matrix.

## Active Runtime Path

### Player pawn chain

- `ASaveThemAllGameMode` sets `DefaultPawnClass = AZhoenusPawn::StaticClass()`.
- `AZhoenusPawn` derives from `ASpaceshipPawn`.
- `ASpaceshipPawn` owns the active Enhanced Input binding code.

Relevant files:

- `Source/Zhoenus/SaveThemAllGameMode.cpp`
- `Source/Zhoenus/ZhoenusPawn.h`
- `Source/Zhoenus/SpaceshipPawn.cpp`
- `Source/Zhoenus/SpaceshipPawn.h`

### Enhanced Input path

`ASpaceshipPawn` does the following:

- Adds `ShipInputMappingContext` to `UEnhancedInputLocalPlayerSubsystem` in `BeginPlay`.
- Binds `ThrustAction`, `PitchAction`, `YawAction`, `RollAction`, `StabilizeAction`, and `FireAction` in `SetupPlayerInputComponent`.
- Stores active analog state in `CachedInput` and `StabilityInput`.
- Applies movement and rotation in `Tick`.

The active ship blueprint contains references to:

- `/Game/Input/DefaultMappingContext`
- `/Game/Input/Actions/ThrustInputAction`
- `/Game/Input/Actions/PitchInputAction`
- `/Game/Input/Actions/YawInputAction`
- `/Game/Input/Actions/RollInputAction`
- `/Game/Input/Actions/StabilizeInputAction`
- `/Game/Input/Actions/MouseXAction`
- `/Game/Input/Actions/MouseYAction`

This confirms that the live pawn path is blueprint-driven on top of the C++ base class.

### Touch path

Touch still bypasses Enhanced Input:

- `AZhoenusPlayerController` creates `UZhoenusTouchUI`.
- Touch buttons call `TouchFirePressed/Released` and `TouchAutoCorrectPressed/Released`.
- Those methods route directly to `FireWeapon` and `OrigDisengageAutoCorrect`.

Relevant files:

- `Source/Zhoenus/ZhoenusPlayerController.cpp`
- `Source/Zhoenus/ZhoenusTouchUI.h`
- `Content/Touch/ZhoenusTouchInput.uasset`

## What Changed in the Recent Gamepad Fix

Commit `4fb60f7` on 2026-03-31 made the following changes:

- Added `FireAction` to `ASpaceshipPawn` and bound firing through Enhanced Input.
- Added `Completed` and `Canceled` bindings for thrust, pitch, yaw, roll, and stabilize so released analog inputs zero out reliably.
- Marked the flight input callbacks as gamepad-sourced by setting `LastInputSource = EInputSource::Gamepad`.
- Updated the input action assets for pitch, thrust, and stabilize. The current action assets show `Axis1D` value types for thrust, pitch, yaw, roll, and stabilize.
- Removed the old controller-side `FireWeapon` axis binding from `AZhoenusPlayerController::SetupInputComponent`.

This matches the commit message: "Fixes for the gamepad controls, we're now using proper axis and analog values."

## Findings

### 1. The active mapping context is `DefaultMappingContext`, not `LoadngoInputMapping`

Evidence:

- `Content/Blueprints/Ships/ZhoenusSpaceShip.uasset` contains `/Game/Input/DefaultMappingContext`.
- Repository-wide binary string search only found `LoadngoInputMapping` inside `Content/Input/LoadngoInputMapping.uasset` itself.

Implication:

- `LoadngoInputMapping` looks like migration debt and should not be treated as part of the live controller path unless a different asset is assigned in-editor later.

### 2. Input is still split across multiple systems

Current split:

- Enhanced Input: ship movement and fire
- Touch UI direct calls: fire and auto-correct
- Legacy `.ini` axis mappings: still present in `Config/DefaultInput.ini`
- RawInput custom device profile: still present in `Config/DefaultInput.ini`
- CommonUI/CommonInput: enabled at the engine level, but not used by the ship input logic

Implication:

- Troubleshooting controller behavior is harder than it needs to be because more than one system can still influence final behavior, depending on device and screen mode.

### 3. Device-source tracking in `ASpaceshipPawn` is too coarse

Current behavior:

- Mouse look sets `LastInputSource = Mouse` in `OnMouseX` and `OnMouseY`.
- Every non-mouse Enhanced Input flight action sets `LastInputSource = Gamepad`.

Why this matters:

- Keyboard actions routed through the same mapping context will also be treated as "gamepad".
- Any future logic based on `LastInputSource` will not be able to distinguish keyboard from controller.
- The mouse virtual-stick decay path is currently keyed off this enum, so mixed keyboard/mouse play can be classified incorrectly.

Relevant code:

- `Source/Zhoenus/SpaceshipPawn.cpp`
- `Source/Zhoenus/SpaceshipPawn.h`

### 4. Deadzone and response behavior are configured in more than one place

Current configuration:

- `Config/DefaultInput.ini` sets stick deadzones to `0.25` for `Gamepad_LeftX`, `Gamepad_LeftY`, `Gamepad_RightX`, and `Gamepad_RightY`.
- `ASpaceshipPawn::Tick` also ignores pitch, yaw, and roll unless absolute input is greater than `0.2`.
- Recent commit history explicitly mentions drift and deadzone tuning:
  - `920ef80 Cleanup roll and yaw.. need to configure dead zones on gamepad`
  - `ebd6b4f ... we may need to configure a 'deadzone' because the HUD skews a little right.`

Implication:

- The project currently has layered thresholds rather than one obvious source of truth.
- That may be fine for now, but it makes controller feel harder to tune and compare across devices.

### 5. The runtime log suggests RawInput should be re-verified against actual hardware

Runtime evidence from `Saved/Logs/Zhoenus.log` dated 2026-03-30:

- RawInput reported a connected device with `VendorID:045E ProductID:02E3`.
- The checked-in RawInput config only defines `VendorID="045e", ProductID="0b13"` in `Config/DefaultInput.ini`.

Inference:

- If the project still depends on RawInput for any controller-specific path, the checked-in device profile may not match the controller currently being tested.
- This may not break the Enhanced Input path directly, but it does mean the RawInput setup should be treated as unverified until tested against the actual hardware IDs in use.

### 6. CommonInput is available and already detects live device changes

Evidence from `Saved/Logs/Zhoenus.log` dated 2026-03-30:

- `LogCommonInput: UCommonInputSubsystem::RecalculateCurrentInputType(): Using Gamepad`
- `LogCommonInput: UCommonInputSubsystem::RecalculateCurrentInputType(): Using Mouse`

Relevant config:

- `Config/DefaultEngine.ini` uses `GameViewportClientClassName=/Script/CommonUI.CommonGameViewportClient`
- `Zhoenus.uproject` enables the `CommonUI` plugin

Implication:

- The engine already knows the active input type at runtime.
- Replacing the pawn-local `LastInputSource` heuristic with CommonInput would reduce guesswork and make mixed-input behavior more reliable.

### 7. `FireAction` is verified in-editor and currently gamepad-only

Runtime evidence from an editor Python inspection on 2026-03-31:

- Ship default object: `/Game/Blueprints/Ships/ZhoenusSpaceShip`
- Active mapping context: `/Game/Input/DefaultMappingContext`
- Fire action: `/Game/Input/Actions/FireInputAction`
- Fire bindings: `Gamepad_RightTriggerAxis`

Implication:

- The current live ship path does receive fire through Enhanced Input as intended.
- Desktop mouse-fire is not present in the active mapping context, so mouse click firing should be treated as missing unless a different path is intentionally supplying it.

### 8. Menu navigation needed native CommonUI focus configuration

UI evidence from 2026-03-31:

- `PowerUpScreenWidget`, `AdjustShip`, and `ConvertSpeed` all use `ZhoenusButton_C` controls.
- Those buttons were authored with:
  - `is_focusable = true`
  - `should_select_upon_receiving_focus = false`
- `ConvertSpeed` also still contained hidden legacy `Button` widgets for the old click-only increment path.

Implication:

- Controller navigation could land inconsistently or fail to present a clear focused-button state even when a widget technically accepted focus.
- `ConvertSpeed` also needed a native focus target so gamepad focus would not resolve to the hidden legacy buttons.

Current fix:

- `UAdjustShipUI`, `UConvertSpeedUI`, and `UPowerUpScreenUI` now treat their live buttons as focus-driven controls rather than selectable/toggle-style controls.
- The important behavior is that the actively focused button reads clearly during controller navigation; CommonUI "selectable" state was not the desired semantic for these menus.
- The native wrappers therefore keep the live buttons focusable and route focus explicitly, without relying on selectable/selected-on-focus behavior.
- Those native classes also provide explicit focus targets.
- `ConvertSpeed` and `PowerUpScreenWidget` now apply explicit directional navigation rules for the visible button layouts.
- `PowerUpScreenWidget` is now reparented to native `UPowerUpScreenUI` so the focus rules are enforced in code instead of depending only on blueprint defaults.
- The widget blueprints for `PowerUpScreenWidget`, `AdjustShip`, and `ConvertSpeed` no longer keep their own `BP_GetDesiredFocusTarget` override, so CommonUI falls back to the native focus logic instead of stale blueprint focus data.
- `PowerUpRoot` is now also reparented to native `UPowerUpRootUI`, marked focusable, set to `auto_activate=true`, and stripped of its old `BP_GetDesiredFocusTarget` override. That wrapper hands focus from the map-level root widget into the active widget on the CommonUI stack instead of leaving focus parked on the root.
- `ZhoenusButton` now has `ZhoenusRepeatButtonStyle` assigned again, and that style asset now defines explicit rounded-box normal and selected brushes. Before that patch the style existed but its brushes were effectively blank, which made focused buttons look unchanged.
- The current expectation is gamepad-first validation of focused-button behavior. Keyboard navigation still needs a later pass before this area should be treated as fully validated across both input paths.
- `PowerUpStatWidget` is now also reparented to native `UPowerUpStatWidgetUI`, and its old event graph plus `InitStatValue` helper graph were stripped from the asset. The row widget is now a layout shell with native behavior only.
- `AdjustShipUI` now seeds the visible stat rows from the loaded `SaveThemAllGameInstance` during `NativeConstruct`, so the menu no longer depends on having played `Level-1` in the same session before showing real stat values.
- `AdjustShipUI` now also updates `CommonNumericTextBlock_PointsRemaining` in native code from the live preview stats. That points-remaining display is no longer just an opaque blueprint subtraction path.
- `AdjustShip` itself also had its event graph stripped, so controller navigation and stat seeding are no longer competing with hidden blueprint activation logic.

Root-cause note:

- The `PowerUp` level script creates `PowerUpRoot`, adds it to the viewport, and calls `SetInputMode_UIOnlyEx`, but it does not explicitly call `ActivateWidget()`.
- Before the latest patch, `PowerUpRoot` was also `auto_activate=false`.
- That meant CommonUI could remain structurally present but inactive, which is why repeated leaf-widget navigation tweaks produced inconsistent or misleading results.
- A second root cause in `AdjustShip` was data-model drift: the UI row labeled `BackwardsAcceleration` had been treated like `MinSpeed`, which belongs to the `Convert` screen. `AdjustShip` now uses a real `ReverseAcceleration` stat for backward thrust, while `Convert` remains responsible for reverse speed limit changes.
- The current `AdjustShip` spend/refund rule is intentionally symmetric and now code-owned in `USaveThemAllGameInstance`. Previewing or saving a lower stat returns points at the same rate that raising that stat consumes them. If design later wants asymmetry, that should be a conscious rules change there rather than a side effect of widget arithmetic.

## Recommended Next Steps

### Priority 1: Consolidate the active control path

- Treat `Content/Input/DefaultMappingContext.uasset` as the source of truth for flight input.
- Audit the checked-in legacy axis mappings in `Config/DefaultInput.ini` and remove or comment the ones that no longer drive active gameplay.
- Decide whether RawInput is still required. If not, remove the custom device profile to reduce ambiguity. If it is required, update it to match current test hardware.
- Remove or archive `Content/Input/LoadngoInputMapping.uasset` if the team confirms it is unused.

### Priority 2: Replace the pawn-local input-source heuristic

- Stop assuming every non-mouse Enhanced Input event is a gamepad event.
- Use `UCommonInputSubsystem` or another engine-level device signal to distinguish mouse, keyboard, and gamepad.
- Keep mouse-stick decay tied to actual current input type rather than the last non-mouse action callback.

### Priority 3: Centralize deadzone and response tuning

- Choose one primary place for deadzone tuning.
- Prefer mapping-context modifiers and action settings over spreading logic between `.ini` and hardcoded thresholds in `Tick`.
- Expose tuning values as data or config so controller feel can be adjusted without recompiling.

### Priority 4: Run a short hardware validation matrix

Minimum matrix:

- Xbox or XInput-compatible controller on Windows
- DualSense or PS-style controller on Windows
- Keyboard and mouse
- Touch UI if mobile remains a supported path

For each device, verify:

- Thrust centers correctly after release
- Pitch/yaw/roll center correctly after release
- No constant drift in HUD bars
- Stabilize trigger behaves analog if intended
- Fire works on the intended button
- Switching between mouse and controller does not leave mouse-look in a bad state

### Priority 5: Record the active bindings explicitly

- Open `DefaultMappingContext` in the editor and capture the exact action-to-key mappings in text.
- Save that list in a follow-up doc or in this file.
- That will remove the remaining guesswork around fire bindings and keyboard parity.

## Suggested Implementation Order

1. Decide whether desktop mouse-fire should exist. If yes, add it to `DefaultMappingContext` or another explicit live path.
2. Verify current controller hardware IDs against the RawInput config.
3. Replace `LastInputSource` with engine-level input-type detection.
4. Move deadzone tuning into one clear place.
5. Remove stale mappings and unused input assets once the live path is confirmed.
6. Keep CommonUI menu focus rules in native code for `PowerUp`, `AdjustShip`, and `ConvertSpeed`, so controller navigation is not dependent on hidden blueprint widgets or non-selecting buttons.

## Open Questions

- Is RawInput still intentionally required for a specific controller, or is XInput/CommonInput enough for current targets?
- Should desktop mouse-fire be restored, or is the current gamepad-only `FireAction` binding intentional?
- Is `LoadngoInputMapping` safe to delete, or is it still referenced from an uninspected map or blueprint?
- Should keyboard and mouse remain first-class controls, or is controller-first the target for this game mode?

## Not Verified in This Pass

- In-editor playtest of the current build
- Full per-action key list inside `DefaultMappingContext` beyond the verified `FireAction` binding
- Whether multiple possessions can stack mapping contexts in practice
- Split-screen or multiple-local-player controller behavior
