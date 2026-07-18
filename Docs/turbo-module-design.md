# Turbo Module Design

Date: 2026-07-18

Status: Approved design direction. Not yet implemented.

## Purpose

This document records the design decision for a purchasable turbo ability in
`Zhoenus`. It captures the options considered, their tradeoffs, and the selected
hybrid of a refundable ship module with a limited per-run charge allowance.

The exact point costs and final acceleration, speed, and cooldown values remain
tuning decisions. The progression and runtime rules described here are the
intended feature contract.

## Core Loop Fit

`Zhoenus` remains centered on the `SaveThemAll` loop:

- fly the ship in full 6DOF space
- gather `DonutFlyers`
- shepherd them through the goal
- earn speed during the run and points at run end
- convert or reallocate ship capability on the power-up screens

Turbo should add a deliberate burst-movement decision to that loop. It should
help with traversal, recovery, and committed goal approaches without making
normal thrust feel inadequate or turning every activation into permanent
progression loss.

## Existing System Boundaries

The design must respect the systems that already exist:

- `points` are durable progression currency earned at run end
- `AdjustShip` supports refundable allocation relative to the saved ship build
- `Convert` reclaims earned forward and reverse speed above the baseline ship
- the battery is currently the recoverable runtime resource for
  `ZapEmProjectiles`
- each saved `DonutFlyer` increases the live pawn's forward and reverse speed
  limits
- the pawn's stored `MaxSpeed` and `MinSpeed` are synchronized back into
  persistent ship stats at run end

Turbo must therefore use a temporary effective speed cap. It must never increase
the stored `MaxSpeed` value directly, because that could accidentally persist a
temporary turbo bonus as a permanent or convertible speed upgrade.

## Design Goals

- Make turbo feel valuable before and during a difficult flight decision.
- Make purchasing turbo part of ship-build expression.
- Give turbo a clear opportunity cost without charging for every activation.
- Keep the battery and projectile economy readable.
- Keep the maximum runtime allowance easy to understand at a glance.
- Support gamepad, keyboard and mouse, and touch play.
- Leave room for future ship abilities and upgrade branches.
- Preserve the current refundable `AdjustShip` philosophy.

## Non-Goals

- Turbo is not a paid entitlement or backend-authoritative unlock.
- Turbo activations do not spend EAB or qcoin rewards.
- Turbo does not consume persistent points during a run.
- Turbo does not consume the projectile battery in the initial design.
- Turbo does not boost reverse thrust or rotational handling.
- The first implementation will not introduce a generic skill-tree framework.
- Turbo does not change scoring or the existing clean-save rules.

## Options Considered

### Option 1: Turbo Consumes The Existing Battery

In this option, holding turbo would continuously consume the same battery used
to fire `ZapEmProjectiles`.

Advantages:

- reuses an existing runtime resource and HUD meter
- creates a direct choice between movement and projectile influence
- fits the longer-term fiction of a ship battery powering several systems
- requires no second charge or heat economy

Disadvantages:

- makes the projectile battery harder to read and balance
- can make players feel that ordinary flight capability is being taxed
- risks making turbo unavailable immediately after the player uses the primary
  follower-control tool
- requires continuous drain and recharge-suppression behavior that the current
  battery does not yet need
- couples two systems before either has received broad balance validation

Decision:

- rejected for the initial turbo design
- may be reconsidered later as part of the broader light-absorption and
  multi-system battery concept

### Option 2: Individual Turbo Charges Purchased With Points

In this option, the player would buy consumable turbo charges on the power-up
screen. A charge would be permanently removed when activated.

Advantages:

- gives points an additional economy sink
- makes each activation feel consequential
- allows players to stock up before a difficult run
- is easy to explain as purchased fuel or cartridges

Disadvantages:

- encourages hoarding instead of experimentation
- turns every activation into permanent ship-build loss
- disproportionately removes a recovery tool from less successful players
- allows successful players to stockpile even more advantage
- requires persistent inventory and purchase-quantity behavior
- complicates balance because level difficulty depends on pre-run stockpiles
- conflicts with the current expectation that ordinary in-run tools do not
  delete durable progression currency

Decision:

- rejected as the baseline economy
- individual consumable charges should not be purchased with power-up points

### Option 3: Permanent One-Time Turbo Unlock

In this option, the player would spend points once and permanently own turbo.

Advantages:

- creates a clear progression milestone
- is simple for the player to understand
- avoids per-use resource costs
- requires little ongoing purchase UI

Disadvantages:

- introduces a non-refundable purchase model beside the refundable ship-build
  model
- needs stronger confirmation and entitlement presentation
- reduces build experimentation after purchase
- makes later balancing or restructuring of the unlock more difficult
- does not create an ongoing tradeoff between turbo and other ship capability

Decision:

- rejected for the initial design
- a permanent unlock may make sense later for account-wide progression, but it
  is not the current ship-build model

### Option 4: Refundable Turbo Module With Heat Or Unlimited Recharge

In this option, points install a refundable module and turbo can be used
repeatedly after an automatically recovering heat or cooldown period.

Advantages:

- fits the existing refundable allocation system
- avoids battery coupling and per-use point spending
- supports upgrades to output, cooling, or capacity
- gives the player freedom to experiment with the ability

Disadvantages:

- an indefinitely renewable turbo can replace normal thrust if its downtime is
  too short
- introduces another continuously changing runtime meter
- makes the total amount of turbo available during a song-length run less
  predictable
- can encourage waiting for recharge instead of continuing to shepherd flyers

Decision:

- accepted in part
- refundable module allocation is retained, but unlimited in-run regeneration
  is replaced by a fixed per-run charge allowance

### Option 5: Refundable Module With Per-Run Charges

In this option, the player allocates points to a turbo module and its upgrade
ranks. The installed module grants a limited number of charges at the start of
each run. Charges do not cost points and do not regenerate during that run.

Advantages:

- preserves the tactical clarity of a maximum of three charges
- gives turbo a real opportunity cost through ship-build allocation
- avoids permanent loss when the player activates the ability
- prevents turbo from replacing normal thrust throughout the whole run
- eliminates persistent consumable inventory and stockpiling
- gives every run a known and testable turbo budget
- fits a small upgrade branch without requiring a full skill-tree system

Disadvantages:

- unused charges do not carry value into the next run
- the player can exhaust the ability early and have no turbo later in the run
- requires a dedicated charge and cooldown HUD state
- requires careful tuning so one charge is useful without three charges becoming
  dominant

Decision:

- selected as the final design direction

## Final Design

### Purchase And Refund Model

Turbo is a refundable ship module purchased with ordinary power-up points in
`AdjustShip`.

Rules:

- the base ship has no turbo module installed
- purchasing the root Turbo Drive rank unlocks the ability
- installed ranks contribute to the current ship allocation cost
- allocated points are unavailable for handling, battery, or other ship stats
- the player may refund the turbo branch between runs
- the root cannot be removed while dependent ranks remain allocated
- the UI may provide a single `Reset Turbo` operation that clears the whole
  branch and refunds its allocation
- preview changes remain provisional until the player saves the ship build
- backing out of `AdjustShip` discards unsaved turbo changes

This is a ship loadout decision, not an account entitlement. A later account or
achievement system may observe module milestones, but it must not become the
authority for local turbo availability.

### Upgrade Branch

The initial data model should support ranks and dependencies even though the
first UI does not need to look like a large visual skill tree.

#### Turbo Drive

- required root rank
- unlocks turbo activation
- grants one charge at the start of each run
- supplies the baseline boost output and cooldown

#### Charge Rack I

- requires Turbo Drive
- raises per-run charge capacity from one to two

#### Charge Rack II

- requires Charge Rack I
- raises per-run charge capacity from two to the hard maximum of three

#### Cooling I And II

- require Turbo Drive
- reduce the activation lockout between charges
- do not regenerate spent charges

#### Overdrive I And II

- require Turbo Drive
- improve forward acceleration and the temporary speed cap
- do not change the fixed three-second duration

The initial implementation may ship with only Turbo Drive and Charge Rack ranks
if that is enough to validate the feature. Cooling and Overdrive are approved
branch directions, not requirements for the first playable pass.

### Point Costs

Exact point costs are intentionally undecided until normal run earnings and
current handling-upgrade costs are measured together.

Cost principles:

- Turbo Drive should be attainable through normal play, not treated as an
  end-game purchase
- reaching three charges should require a meaningful commitment compared with
  acceleration and handling upgrades
- higher ranks should use explicit rank costs rather than fractional per-unit
  stat ratios
- refund value should equal allocation cost under the current symmetric
  `AdjustShip` rules
- changing costs later must not strand or duplicate allocated points in existing
  saves

### Per-Run Charge Rules

- a run begins with charges equal to the installed charge capacity
- the base Turbo Drive supplies one charge
- the hard maximum is three charges
- activating turbo consumes one whole charge immediately
- charges do not regenerate during the run
- charges are not saved as persistent inventory
- unused charges do not carry over or convert back into points
- the next run refills the allowance to the installed capacity
- changing the installed capacity only occurs between runs through `AdjustShip`

### Activation Rules

- turbo is a discrete action, not an analog strength
- pressing the action commits one charge to a three-second burst
- an activation cannot begin without an installed module, an available charge,
  and a completed cooldown
- turbo supplies forward acceleration along the ship's current forward vector
- the player may continue to pitch, yaw, and roll during the burst
- turbo never applies reverse acceleration
- reverse-thrust input does not turn turbo into a reverse boost
- the player cannot cancel an activation to recover the spent charge
- a blocking collision or planet guardrail intervention may end the active burst
  early, but the charge remains spent

A committed burst is preferred over hold-to-drain behavior because every charge
has a predictable value across gamepad, keyboard, and touch input.

### Initial Runtime Tuning Target

The first playable pass should start near these values:

- burst duration: `3.0` seconds
- forward acceleration multiplier: `2.0`
- temporary forward speed-cap multiplier: `1.30`
- base lockout after a burst: `4.0` seconds
- base charges per run: `1`
- maximum charges per run: `3`

These are test values, not balance promises. Config or data should own them so
they can be tuned without changing the progression contract.

### Speed And Persistence Safety

Turbo must not mutate the persistent or convertible speed values.

Required behavior:

- compute a temporary effective cap from the pawn's current stored `MaxSpeed`
- allow the pawn to exceed normal `MaxSpeed` only while the turbo rules permit it
- when turbo ends, return excess speed smoothly toward normal `MaxSpeed`
- do not snap the pawn immediately from turbo speed to normal speed
- do not write the temporary cap into `FShipStats`
- preserve the existing per-save `MaxSpeed += 5` and `MinSpeed -= 2` rewards
- allow the turbo multiplier to operate on the legitimate live forward cap,
  including speed earned from saved flyers
- ensure run-end speed synchronization sees only the legitimate stored cap

### Cooldown

The cooldown is an activation lockout, not charge regeneration.

- cooldown begins when the active three-second burst ends
- another available charge cannot activate until cooldown completes
- cooldown state resets appropriately when a new run begins
- Cooling upgrades reduce the lockout but never make simultaneous or overlapping
  turbo activations possible
- reaching zero charges makes cooldown irrelevant for the remainder of the run

### Battery Separation

Turbo does not consume `UZhoenusBatteryComponent` energy in the initial design.

This preserves clear system meanings:

- battery controls current `ZapEmProjectile` availability
- turbo pips show the run's remaining burst allowance
- cooldown shows when the next owned charge may activate
- points control the installed ship build between runs

The systems may be coupled in a later multi-system energy design, but that would
be a separate design change rather than an implementation shortcut.

## UI And HUD Direction

### AdjustShip

The first UI should present a focused Turbo Module section within `AdjustShip`.

It must show:

- whether Turbo Drive is installed
- current and maximum rank for each available upgrade
- the point cost or refund for the pending change
- unmet dependency state for locked child ranks
- the same live points-remaining preview used by the rest of the ship build
- clear controller focus and navigation behavior

The first implementation should use ranked controls and dependency data, but it
should not build a generic node-graph editor for one module. When multiple ship
abilities have real branches, the same rank data can support a dedicated visual
skill-tree screen.

### Flight HUD

Turbo needs one compact, platform-aware truth source on the flight HUD:

- one pip per installed charge, up to three
- a spent charge is visibly unavailable
- the next available charge shows cooldown progress when locked
- active turbo has a distinct but restrained state
- the turbo display must not be confused with the battery meter
- mobile placement must avoid touch-control regions and safe-zone cutouts

The HUD should not show turbo controls before the module is installed unless a
locked-state preview is intentionally part of the progression presentation.

### Input

Turbo requires a dedicated Enhanced Input action and equivalent touch path.

Input requirements:

- one digital activation action
- explicit press handling so one press cannot consume multiple charges
- no dependency on analog trigger magnitude
- a gamepad binding that does not interfere with thrust, fire, or stabilize
- a keyboard binding in the active mapping context
- a touch button with press behavior, not the current Fire or Stabilize toggle
  behavior
- release, cancel, possession, and level-transition handling that cannot leave
  turbo active or consume an extra charge

Exact physical bindings should be finalized while auditing the live
`DefaultMappingContext` in the editor.

## Presentation Direction

The gameplay state must be proven before presentation polish, but turbo should
eventually drive:

- stronger output from the two attached aft `ShipEngineFlare` effects
- a distinct flare hue or intensity that reads as turbo rather than ordinary
  thrust
- a concise activation sound and clear end or cooldown feedback
- restrained camera feedback that does not undermine aiming or cause motion
  discomfort

`ShipEngineFlare` is already a Niagara system attached to the ship. The separate
`ShipFlareEmitter` asset is an emitter, not a spawnable Niagara system, and
should not be treated as the runtime turbo effect.

## Gameplay Interactions

### DonutFlyers And Goals

- turbo does not directly alter follower aggro or goal-lock behavior
- higher approach speed may make shepherding and goal alignment harder
- turbo should remain a risk-reward movement tool, not an automatic save action
- playtesting must verify that boosted approaches do not make followers
  unrecoverable or expose gate trap geometry

### Clean Saves

- using turbo does not itself invalidate a clean save
- stabilize remains the explicit clean-save disqualifier at goal crossing
- collisions during turbo reset the clean-save attempt under the existing rules
- a turbo collision ending the burst does not refund the charge

### Scoring And Convert

- turbo activation awards no points
- unused charges award no points
- turbo does not change end-of-run payout multipliers
- temporary turbo speed is not convertible
- legitimate speed earned from saved flyers remains convertible under the
  existing rules

## Persistence Model

Persistent state should store integer module ranks, not transient run state or
derived tuning values.

Conceptual persistent fields:

- Turbo Drive rank
- Charge Rack rank
- Cooling rank
- Overdrive rank

Conceptual transient fields:

- remaining charges for the current run
- whether a burst is active
- remaining burst time
- remaining cooldown time

Legacy saves should load with every turbo rank at zero. Invalid ranks should be
clamped, and child ranks without their required parent should be repaired or
cleared deterministically.

## Implementation Staging

### Stage 1: Minimal Playable Turbo

- Turbo Drive root rank
- one charge per run
- three-second committed burst
- temporary acceleration and speed cap
- four-second cooldown contract
- safe overspeed recovery
- input on the primary test platform
- debug visibility for charge and cooldown state

### Stage 2: Progression And HUD

- refundable allocation in `AdjustShip`
- Charge Rack I and II
- persistent rank migration and validation
- desktop and mobile charge HUD
- gamepad, keyboard, mouse, and touch validation

### Stage 3: Tuning And Presentation

- evaluate Cooling and Overdrive ranks
- tune costs against real run payouts
- tune follower and goal-approach behavior
- drive attached engine flares
- add audio and restrained camera feedback

### Stage 4: Broader Ability Tree, Only When Justified

- introduce a generic skill-tree presentation only after multiple abilities have
  meaningful dependencies
- preserve the rank and dependency data from the Turbo Module branch
- avoid rewriting working progression rules solely to obtain a tree-shaped UI

## Acceptance Criteria

- A ship without Turbo Drive cannot activate turbo or spend a runtime charge.
- Installing Turbo Drive deducts its allocation cost only when the build is
  saved.
- Refunding the branch restores its allocation under the current symmetric
  point rules.
- A run starts with the correct installed charge capacity.
- One input press consumes exactly one charge and produces one three-second
  burst.
- No more than three charges can be available in a run.
- Cooldown prevents overlapping activations.
- Charges do not consume battery or points during flight.
- Turbo never mutates or persists the temporary speed cap as `MaxSpeed`.
- Excess turbo speed returns smoothly to the normal cap.
- Blocking collisions and planet guardrail intervention cannot leave turbo
  latched active.
- Turbo status is readable on desktop and touch layouts without obscuring the
  reticle or battery.
- Gamepad, keyboard and mouse, and touch each activate turbo once per intended
  press.
- Existing firing, stabilize, scoring, clean-save, convert, and save-game flows
  continue to work.

## Final Decision Summary

Turbo will be a refundable ship module with ranked upgrades. Installing the root
module grants one automatically refilled charge at the beginning of every run.
Capacity upgrades may raise that allowance to a hard maximum of three. Each
activation consumes one run-local charge and commits the ship to a fixed
three-second forward boost, followed by an activation cooldown. Charges do not
regenerate during the run, do not consume the projectile battery, do not cost
points when used, and do not persist as inventory.

The upgrade data should support dependencies like a skill tree, but the first UI
will remain a focused Turbo Module section inside `AdjustShip`. A generic visual
skill tree will be considered only when additional abilities justify it.
