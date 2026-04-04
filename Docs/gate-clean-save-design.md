# Gate Clean-Save Design

Date: 2026-03-31

## Problem

The current Gate of Oblivion mesh has twisted windings that create false openings and interior pockets. Players and DonutFlyers can enter those spaces and then fail to escape cleanly. That produces the wrong gameplay result:

- a follower is neither rejected nor saved
- the player can commit to the gate and get trapped in decorative geometry
- AI lock logic can keep steering toward a goal that is technically reachable in space but not reachable through the collision envelope

This is not only an AI tuning issue. It is also a gate geometry and collision-shape issue.

## Clean-Save Rule

The gate must either reject or save. It must never trap.

That means:

- the intended tunnel through the gate must be obvious and traversable
- decorative windings must not expose enterable cavities
- collisions on the decorative structure must push actors away instead of admitting them into dead spaces
- the scoring trigger should live in the true pass-through path, not inside ambiguous mesh volume

## Separation Of Concerns

The gate should be treated as three different layers instead of one asset doing everything.

### 1. Art Mesh

The visible gate can remain visually rich, twisted, and strange. It is there to sell the fiction and silhouette of the Gate of Oblivion.

### 2. Blocking Collision

Gameplay collision should be simpler than the art mesh.

Recommended shape:

- one clean central tunnel or throat that actors are allowed to pass through
- simple blocker volumes around the windings
- no concave pockets that can admit a player or follower and then hold them

If a player clips the outer structure, they should glance off or be rejected, not get threaded into the windings.

### 3. Save Trigger

The actual scoring trigger should be a separate volume placed in the valid pass-through path.

Recommended placement:

- in the tunnel throat or slightly beyond it
- aligned with the intended line of travel
- not dependent on touching decorative surfaces

## AI Implication

The current goal-lock work improved follower behavior by aiming at the live goal bounds origin instead of a stale impact point. That was necessary, but it is not enough if the gate still contains trap geometry.

For a clean save:

- the follower lock target should align with the real tunnel center
- the tunnel must be reachable without crossing decorative blockers
- any false opening exposed by the art mesh will still create bad lock behavior, even with better steering

So the next level of reliability comes from collision layout, not only controller tuning.

## Implementation Guidance

When the gate is rebuilt or wrapped with helper volumes, keep these priorities:

1. Preserve the visual windings for art.
2. Replace or override mesh collision with intentional simple volumes.
3. Define one unmistakable save corridor.
4. Use blocker volumes to seal every decorative pocket.
5. Keep the save trigger separate from the art mesh and separate from blocker volumes.

## Test Checklist

- A player approaching off-center gets pushed away from the windings instead of entering them.
- A player approaching through the center can pass through without snagging.
- A follower locked to the gate either crosses the trigger cleanly or is deflected away.
- No actor can remain lodged inside the windings.
- The gate feels readable: line up, commit, save.

## Collaboration Note

Anyone helping this part of the project should treat "clean save" as the acceptance criterion. A visually accurate gate that still admits trap pockets is not finished. A simpler invisible collision shell that produces consistent saves is the correct direction.
