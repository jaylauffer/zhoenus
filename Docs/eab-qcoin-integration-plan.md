# Zhoenus EAB + QCoin Integration Plan

Date: 2026-04-04

## Reviewed Context

Zhoenus docs and runtime files reviewed for this plan:

- `README.md`
- `Docs/AGENT_BOOTSTRAP.md`
- `Docs/point-system-and-convert.md`
- `Source/Zhoenus/SaveThemAllGameInstance.h`
- `Source/Zhoenus/SaveThemAllGameInstance.cpp`
- `Source/Zhoenus/SaveThemAllGameMode.cpp`
- `Source/Zhoenus/SaveThemAllPlayerController.cpp`
- `Source/Zhoenus/AdjustShipUI.cpp`
- `Source/Zhoenus/ShipStats.h`
- `Source/Zhoenus/Zhoenus.Build.cs`

External public repo docs reviewed for system roles:

- `qcoin` public `README.md`
- `entitlement-achievement-blockchain` public `README.md`

## Gameplay Loop Guardrail

`Zhoenus` is a UE5 6DOF spaceship game centered on `SaveThemAll`:

- fly the ship
- gather `DonutFlyers`
- guide them through the goal
- earn an end-of-run point payout
- spend or convert those gains through the power-up flow

Any EAB or qcoin integration must reinforce that loop, not replace it with account friction, wallet UI, or network-dependent progression.

## Current Runtime Truth

Today the gameplay economy is local-first:

- `USaveThemAllGameInstance` is the persistent authority for:
  - `shipStats`
  - `points`
  - `Saved`
  - `AcquiredPoints`
  - `SpentPoints`
  - `ConvertedPoints`
  - `TotalAttempts`
  - `TotalSuccess`
- `USaveThemAllGameInstance::SaveGame()` persists that state through `USaveThemAllSaveGame`.
- `ASaveThemAllGameMode::EndPlay()` computes the end-of-run payout, increments `TotalAttempts`, and saves.
- `ASaveThemAllPlayerController::OnUnPossess()` syncs the ship's live speed back into saved ship stats.
- `UAdjustShipUI::HandleSaveClicked()` currently writes adjusted ship stats and remaining points directly into the game instance, then saves.

Important current limitation:

- `Source/Zhoenus/Zhoenus.Build.cs` does not yet include `Http`, `Json`, or `JsonUtilities`.
- `Source/Zhoenus/` does not currently contain an HTTP client, JSON serialization layer, or remote identity/session code.

This means backend integration is still a design task, not an in-progress runtime path.

## External System Roles

The public repos define different responsibilities:

- `entitlement-achievement-blockchain` is the gameplay-facing service.
  - It already models identity exchange, player profiles, achievement claims, authoritative achievement awards, entitlement grants, and optional qcoin mirroring.
- `qcoin` is the ledger and node layer.
  - It provides the chain, block sync, validator config, and multi-node operation.

That division matters for Zhoenus.

## Core Integration Decision

Zhoenus should integrate with EAB directly.

Zhoenus should not integrate with qcoin directly for normal gameplay.

Reason:

- EAB matches game-facing needs: identity, claims, achievements, entitlements, receipts.
- qcoin is infrastructure, not a client gameplay SDK.
- Phones, consoles, and Switch targets should not be expected to run a node, manage validator state, or block gameplay on ledger availability.

The intended stack is:

- Zhoenus client -> EAB service
- EAB service -> optional qcoin mirroring

## Platform Goal

This integration should remain viable for:

- PC
- Mac
- phones
- consoles
- Switch

That implies these guardrails:

- local play must remain responsive without a network round trip
- local save data must remain usable offline
- network sync must be asynchronous and retryable
- trusted-service tokens must never ship in the public game client
- qcoin node operation must remain server-side or lab-only, not a retail client requirement

## Security Boundary

The public client may use player-facing EAB endpoints.

The public client must not hold broad developer tokens.

So the safe split is:

- client-side:
  - identity exchange
  - session-backed profile reads
  - player-submitted achievement claims or telemetry-style event submission
- trusted service-side:
  - authoritative achievement awards
  - entitlement grants
  - any qcoin-mirrored reward receipts

If Zhoenus wants authoritative progression, a thin trusted Zhoenus service should sit beside EAB or in front of the privileged EAB endpoints.

## Proposed Client Architecture

Add a backend sync layer owned by the game instance rather than by a single level or widget.

Recommended shape:

- `UGameInstanceSubsystem` for remote identity, session state, entitlement cache, and queued sync work
- lightweight C++ DTO structs for outbound run/spend/convert events
- a persistent outbox so finished runs are not lost when offline

Why a game-instance-owned subsystem:

- it survives map changes
- it matches the current `USaveThemAllGameInstance` ownership model
- it can observe both gameplay-end events and power-up screen mutations

## Proposed State Split

Keep these as local-authoritative in Zhoenus:

- live ship handling
- live save count during a run
- run payout calculation
- local ship stat changes
- local convert behavior

Mirror these to EAB asynchronously:

- run completed summaries
- lifetime stat deltas
- ship-build updates that represent durable progression
- convert events
- milestone claims
- entitlement unlock requests

Treat these as remote-authoritative once the service path exists:

- achievements
- entitlements
- cross-device reward receipts
- any public or semi-public qcoin-backed proof

## First Integration Hook Points

The current code already provides reasonable places to attach sync without disturbing flight feel.

### 1. Session bootstrap

Hook near startup after local save load:

- `USaveThemAllGameInstance::Init()`
- `ASaveThemAllGameMode::BeginPlay()`

Responsibility:

- restore cached identity/session state
- begin async identity exchange or refresh
- pull remote profile and entitlements without blocking level start

### 2. Run completion event

Hook:

- `ASaveThemAllGameMode::EndPlay()`

Payload should include:

- run id
- timestamp
- `Saved`
- `Total`
- percentage saved
- payout awarded
- resulting local `points`
- resulting `shipStats.MaxSpeed`
- resulting `shipStats.MinSpeed`

This is the most important first sync path.

### 3. Convert event

Hook:

- `USaveThemAllGameInstance::ConvertMaxSpeed()`

Payload should include:

- forward speed converted
- reverse speed converted
- points minted from the conversion
- resulting local point total

### 4. Ship-build update

Current state:

- `UAdjustShipUI::HandleSaveClicked()` writes `shipStats` and `points` directly

That is acceptable locally, but it is the wrong long-term shape for mirrored progression.

Before remote progression goes live, ship-build saves should funnel through a centralized game-instance method so one place can:

- validate the mutation
- update lifetime accounting if needed
- persist locally
- enqueue a mirrored remote event

## Event Model

The first pass should stay narrow.

Recommended outbound event types:

- `run_completed`
- `speed_converted`
- `ship_build_saved`
- `achievement_claim_submitted`

Do not start with:

- per-flyer events
- per-frame telemetry
- live combat events
- direct qcoin transaction creation from the client

`SaveThemAll` is a loop game, not an event firehose product.

## Offline-First Sync Model

The sync path should use an outbox model:

1. local gameplay mutation happens first
2. local save is updated immediately
3. a durable outbound event record is appended
4. the backend subsystem attempts delivery
5. on success, the outbox item is marked delivered
6. on failure, retry later

The natural first place to persist that outbox is an extension of `USaveThemAllSaveGame` or a second save object owned by the same game instance.

Required behaviors:

- duplicate delivery must be safe
- events need stable ids
- game shutdown during sync must not lose completed runs
- no level flow should wait on remote acknowledgment

## Entitlement Direction

EAB is a better fit for durable unlocks than for frame-critical economy.

Good early entitlement candidates:

- cosmetic unlocks
- ship skins or visual effects
- soundtrack/video gallery access
- milestone unlock markers

Bad early entitlement candidates:

- live thrust values
- per-run speed bonus application
- immediate flight handling changes during a level

If a network outage can make the ship feel wrong, the integration boundary is in the wrong place.

## Achievement Direction

Use EAB's claim/review split instead of letting the public client self-award.

Good first achievement candidates:

- first completed run
- first perfect save
- lifetime flyer milestones
- first convert action
- first ship-build save after spending points

Client role:

- submit the claim context

Trusted service role:

- validate
- promote to an authoritative award
- grant any linked entitlement
- optionally mirror the award into qcoin

## QCoin Role in Zhoenus

qcoin should stay behind EAB for normal player-facing integration.

Useful qcoin-backed outcomes:

- mirrored proof that an achievement was authoritatively awarded
- mirrored proof that an entitlement exists
- future public receipts for tradable or portable digital items

Not recommended for the first Zhoenus integration pass:

- running qcoin-node inside the game client
- making run completion depend on chain availability
- turning current point payout directly into on-chain currency

`points` in Zhoenus are currently gameplay tuning currency. That should remain true until there is a deliberate economy redesign.

## Implementation Phases

### Phase 1: Identity + Run Receipt Mirroring

Add:

- HTTP/JSON dependencies
- game-instance backend subsystem
- local outbox
- EAB identity exchange
- `run_completed` sync

Goal:

- preserve the full local loop
- prove that completed runs can mirror to a player profile without harming play

### Phase 2: Convert + Ship Build Sync

Add:

- centralized ship-build save path
- `speed_converted` sync
- `ship_build_saved` sync

Goal:

- make durable progression reconstructable across devices

### Phase 3: Claims, Awards, and Entitlements

Add:

- player claim submission
- trusted-service review/award flow
- entitlement pull/cache in the client

Goal:

- let Zhoenus unlock durable cross-device rewards without trusting the public client with privileged mutation tokens

### Phase 4: Optional qcoin-backed Receipts

Add:

- EAB qcoin mirroring for authoritative records worth publishing or verifying

Goal:

- gain ledger-backed receipts without polluting the game client with blockchain responsibilities

## Non-Goals For Now

- replacing local save data with remote authority
- requiring a login before the player can fly
- putting qcoin-node on phone or console clients
- tying frame-critical ship behavior to backend latency
- redefining Zhoenus points as an on-chain asset

## Immediate Follow-Up Recommendation

Before code is written, the next implementation doc should define:

- the exact UE-side subsystem/class names
- the outbox record schema
- the `run_completed` payload format
- how local player identity is mapped into EAB
- which rewards are read-only vs remote-authoritative in the first shipping pass

That is the smallest concrete design package that can move this from discussion into implementation without damaging the current `SaveThemAll` loop.
