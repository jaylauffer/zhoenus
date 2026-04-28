# Zhoenus Active Task

Revised: 2026-04-27

Update this file as priorities change.

## Current Focus

Bring up the Level-1 talking-head assistant while keeping the achievement/EAB
contract separate.

The `SaveThemAll` loop under discussion is:

- fly the ship in 6DOF space
- gather `DonutFlyers`
- shepherd them through the goal
- earn end-of-run points
- spend or convert ship capability on the power-up screens

The integration goal is to mirror selected accomplishments to EAB without
damaging local-first gameplay.

The immediate gameplay-facing goal is to use the `TalkingHeadBase` actor in
`Level-1` as the anchor for Jay's local assistant: an image projected above the
metallic object, player-facing fallback guidance, and a clean natural-language
seam that can later connect to a local `gpt-oss` agent.

## Current Product Boundary

Zhoenus should:

- keep local gameplay and save data usable offline
- record local claim/outbox entries for meaningful accomplishments
- submit pending claims to EAB when connectivity exists
- read authoritative rewards or receipts from EAB later

Zhoenus should not:

- talk to qcoin directly for normal gameplay
- ship trusted-service EAB tokens in the public client
- block run completion or power-up flow on backend availability
- treat local claims as authoritative awards

## Required Reading

Before editing Zhoenus achievement or EAB integration code, read:

- `README.md`
- `Docs/AGENT_BOOTSTRAP.md`
- `Docs/eab-qcoin-integration-plan.md`
- `/Users/jay/pudding/docs/ZHOENUS_EAB_CLAIMS.md`
- `/Users/jay/pudding/docs/ZHOENUS_ACHIEVEMENT_CATALOG.md`
- `Source/Zhoenus/SaveThemAllGameInstance.h`
- `Source/Zhoenus/SaveThemAllGameInstance.cpp`
- `Source/Zhoenus/SaveThemAllGameMode.cpp`
- `Source/Zhoenus/ConvertSpeedUI.cpp`
- `Source/Zhoenus/AdjustShipUI.cpp`
- `Source/Zhoenus/SpaceshipPawn.cpp`
- `Source/Zhoenus/Zhoenus.Build.cs`

Also read the specific gameplay docs if a claim touches that system:

- `Docs/point-system-and-convert.md`
- `Docs/game-design-concepts.md`
- `Docs/donutflyer-aggro-design.md`
- `Docs/gate-clean-save-design.md`
- `Docs/hud-current-state.md`

## Immediate Priorities

1. Confirm the first claim-producing runtime events:
   - run completed
   - flyers saved
   - perfect save
   - speed converted
   - ship build saved
2. Confirm whether Zhoenus currently has HTTP/JSON dependencies or backend sync
   scaffolding.
3. Define a local outbox shape that survives offline play and retry.
4. Keep the first implementation event-level, not telemetry-heavy.
5. Wire EAB only after the EAB fixtures and claim/award path are verified.

## Talking Head Priorities

1. Auto-spawn the native assistant above the actor named or tagged
   `TalkingHeadBase`.
2. Keep the first assistant useful without a live model by reading current
   `SaveThemAll` state.
3. Expose a natural-language seam for later local agent integration.
4. Keep face/diffusion/model assets configurable rather than hard-coded into
   gameplay logic.
5. Do not make AI availability a requirement for run completion.

## Current Runtime Truth To Verify

Existing docs indicate:

- `ASaveThemAllGameMode::EndPlay()` computes end-of-run payout and saves.
- `USaveThemAllGameInstance` owns persistent points, ship stats, and convert
  logic.
- `USaveThemAllGameInstance::ConvertMaxSpeed()` is the likely convert-event
  hook.
- `UAdjustShipUI` owns native ship-build save behavior for adjusted stats.
- `ASpaceshipPawn::FireShot()` now uses battery energy for projectile firing.
- The older integration plan said `Zhoenus.Build.cs` did not yet include
  `Http`, `Json`, or `JsonUtilities`; verify this before coding.

Do not assume older notes are still accurate without checking source.

## Prior Gameplay Active Task

The previous active Zhoenus task was `Level-1` planet-boundary validation:

- `APlanetBody` is the authority.
- `ASpaceshipPawn` enforces the guardrail.
- `APlanetSurfaceRuntime` is only visual continuation.

That remains valid handoff context for boundary work, but it is not the current
Pudding-level focus unless the task explicitly returns to `Level-1`
containment.

## Do Not Drift Into

- wallet UX
- qcoin client integration
- public economy design
- live-device assumptions
- high-frequency telemetry
- achievement definitions hard-coded into EAB runtime code

## Expected Agent Behavior

- Start by restating the relevant `SaveThemAll` loop.
- Name the docs and source files read before editing.
- Keep claims non-authoritative.
- Keep EAB as the authority for awards and entitlements.
- Keep qcoin downstream of EAB, not in the game client.
- Update this file if the active focus changes.
