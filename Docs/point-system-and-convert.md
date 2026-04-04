# Zhoenus Point System and Convert Flow

Date: 2026-03-31

## Scope

This note records the current `SaveThemAll` reward flow and the restored `Convert` behavior so the power-up screen, ship stats, and save data all describe the same model.

## Current Point Flow

Points are awarded at the end of the run, not per flyer.

Relevant runtime file:

- `Source/Zhoenus/SaveThemAllGameMode.cpp`

Current behavior in `ASaveThemAllGameMode::Score`:

- Each flyer sent through the goal increments the round `Saved` count.
- Each save also boosts live ship speed:
  - `MaxSpeed += 5`
  - `MinSpeed -= 2`

Current behavior in `ASaveThemAllGameMode::EndPlay`:

- The round payout is calculated from the number of saves and the save ratio.
- If the player saves every flyer, payout is:
  - `Saved * 1.45`
- Otherwise:
  - more than 74% saved: `Saved * 1.25`
  - more than 30% saved: `Saved * 1.0`
  - 30% or less saved: `Saved * 0.9`
- The payout is then added to:
  - `USaveThemAllGameInstance::points`
  - `USaveThemAllGameInstance::AcquiredPoints`

This means the live speed bonuses earned during play and the point payout are separate systems:

- speed bonuses are earned immediately per save
- spendable points are paid at run end

## Restored Convert Behavior

Relevant runtime files:

- `Source/Zhoenus/SaveThemAllGameInstance.h`
- `Source/Zhoenus/SaveThemAllGameInstance.cpp`
- `Source/Zhoenus/SaveThemAllPlayerController.cpp`

The restored convert flow treats only speed above the baseline ship limits as convertible.

Baseline stored ship limits:

- `MaxSpeed = 3000`
- `MinSpeed = -1000`

Convertible speed is defined as:

- forward convertible speed: `shipStats.MaxSpeed - 3000`
- reverse convertible speed: `-1000 - shipStats.MinSpeed`

Both values are clamped to zero, so the player cannot convert below the default ship.

### Conversion ratio

`USaveThemAllGameInstance::Convert` now uses:

- `1 point per 7 total speed`

That is represented in code as:

- `SpeedConversionRatio = 1.0 / 7.0`

This matches the live save bonus shape:

- one save currently adds `+5` forward speed and `-2` reverse speed
- converting that combined `7` speed back out yields `1` point

### Convert operation

`USaveThemAllGameInstance::ConvertMaxSpeed(ForwardAmount, ReverseAmount, bPersist)` now:

1. clamps the requested forward and reverse conversion against the currently convertible speed
2. subtracts the converted forward amount from `shipStats.MaxSpeed`
3. adds the converted reverse amount back toward zero on `shipStats.MinSpeed`
4. adds the resulting point total to `points`
5. adds the same point total to `ConvertedPoints`
6. optionally saves immediately

This gives the power-up UI a safe code path for reclaiming temporary speed into spendable points without letting the ship fall below the base configuration.

## Persistence

The live ship still earns speed boosts on the pawn during the level. Those speed values need to survive the transition into the power-up screen.

`ASaveThemAllPlayerController::OnUnPossess` now calls:

- `USaveThemAllGameInstance::SyncShipSpeedStats`

That helper writes the pawn's current:

- `MaxSpeed`
- `MinSpeed`

back into the saved `shipStats` struct, which is the same state the power-up UI reads.

This is what makes the restored convert flow meaningful:

- run grants speed on the pawn
- speed is synced back into saved ship stats when the run ends
- convert operates on those saved ship limits
- save game persists the result for the next run

## AdjustShip Separation

`AdjustShip` is no longer overloaded with convert-speed state.

Persistent handling stats now include:

- `ForwardAcceleration`
- `ReverseAcceleration`
- `PitchAcceleration`
- `YawAcceleration`
- `RollAcceleration`

`ASpaceshipPawn` now reads both `ForwardAcceleration` and `ReverseAcceleration` from saved ship stats in `BeginPlay`:

- forward thrust uses `ForwardAcceleration`
- backward thrust uses `ReverseAcceleration`
- `MinSpeed` remains the reverse speed cap, not the reverse thrust stat

Current default thrust baselines:

- `ForwardAcceleration = 500`
- `ReverseAcceleration = 200`

`UAdjustShipUI` now:

- seeds each stat row from `USaveThemAllGameInstance::shipStats` during `NativeConstruct`
- seeds them again during `NativeOnActivated`, with no blueprint stat-init path left in the asset
- saves the reverse row back into `shipStats.ReverseAcceleration`
- leaves `shipStats.MinSpeed` alone so the `Convert` screen remains the only place that adjusts speed caps
- computes `PointsRemaining` from native code rather than relying on implicit widget math
- uses `UPowerUpStatWidgetUI` as the source of truth for the visible stat-row value, so preview math reads the same numbers the player is actually changing
- restores the visible stat-row label from the widget blueprint `DisplayLabel` property, so the layout asset still controls the user-facing stat name
- no longer depends on `AdjustShip` or `PowerUpStatWidget` event-graph stat helpers for initialization or stat math

### AdjustShip point rule

`AdjustShip` now has an explicit native point model in `USaveThemAllGameInstance`.

Current rule:

- each point of stat increase above the default ship baseline costs points according to that stat type's per-unit ratio
- each point of stat decrease back toward that baseline refunds points according to that same per-unit ratio

Current temporary per-unit ratios:

- `ForwardAcceleration`: `0.32`
- `ReverseAcceleration`: `0.18`
- `PitchAcceleration`: `0.07`
- `YawAcceleration`: `0.10`
- `RollAcceleration`: `0.08`

This is symmetric on purpose for now.

The budget is computed as:

- current spendable points
- plus the allocation cost already embodied in the saved ship build

The preview state then subtracts its own current allocation cost from that budget. That means:

- opening `AdjustShip` on an already-upgraded ship still lets the player reshuffle those invested points
- lowering a stat gives those points back immediately at the same rate
- the row `+` / `-` buttons continue to move by one raw stat unit, and the point delta is fractional according to the ratio table above
- saving writes both the adjusted stats and the remaining points back to the save

This is a design choice, not an accident. If the game should later penalize downgrades or otherwise make refunds asymmetric, that should be introduced explicitly as a new rule in `USaveThemAllGameInstance`.

### Legacy save migration note

Older save files predate `ReverseAcceleration`.

Those saves deserialize that field as `0`, which is not a meaningful migrated gameplay value for the current handling model. `USaveThemAllGameInstance::LoadGame` now repairs that case by restoring the baseline reverse thrust value during load.

This separation matters because the two screens now map cleanly to different systems:

- `AdjustShip`: handling and thrust characteristics
- `Convert`: reclaiming temporary forward/reverse speed limit gains into points

## Debug Point Grant Command

Relevant runtime file:

- `Source/Zhoenus/ZhoenusPlayerController.cpp`

For debug work, the shared player controller now exposes a console command:

- `GrantPowerPoints <amount>`

This command is compiled only for non-shipping, non-test builds.

Example:

- `GrantPowerPoints 25`

Behavior:

- adds the requested amount directly to `USaveThemAllGameInstance::points`
- also increments `USaveThemAllGameInstance::AcquiredPoints` so lifetime totals stay coherent
- saves immediately through `USaveThemAllGameInstance::SaveGame()`
- prints the resulting total through the console/client message path

This is intended only as a debug convenience so `PowerUp`, `AdjustShip`, and `Convert` can be tested without first earning a payout through `Level-1`.

## Follow-up Work

- `ConvertSpeed` now uses a native `UConvertSpeedUI` parent plus styled `ZhoenusButton` increment/decrement controls.
- Those increment/decrement buttons are initialized from the actual convertible forward and reverse speed amounts exposed by `USaveThemAllGameInstance`.
- The buttons now repeat while held and play the shared button click sound on each repeat step, matching the older `ZhoenusRepeatButton` interaction pattern more closely than plain click-only buttons.
- `UConvertSpeedUI::NativeGetDesiredFocusTarget` now returns a live visible increment/decrement button so gamepad focus does not land on the old hidden button widgets left in the blueprint for graph compatibility.
- `UConvertSpeedUI` also marks the live `ZhoenusButton` controls as selectable-on-focus and applies explicit left/right/up/down navigation so controller input can move across the forward and reverse convert rows reliably.
- `AdjustShip` point accounting is now explicit in `USaveThemAllGameInstance`, not implicit blueprint subtraction. If design later wants asymmetric downgrade refunds, introduce that deliberately there instead of burying it in widget math.
