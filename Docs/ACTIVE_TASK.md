# Zhoenus Active Task

Update this file as priorities change.

## **ACTIVE_TASK.md**
**Revised 2026-04-26**
**Last Updated by: codex**

### **Current Focus**
Validate the simple planet-boundary model in `Level-1`:

- `APlanetBody` is the authority
- `ASpaceshipPawn` enforces the guardrail
- `APlanetSurfaceRuntime` is only the cheap visual continuation

The `SaveThemAll` loop is unchanged:

- fly the ship in open 6DOF space
- gather `DonutFlyers`
- shepherd them through the goal
- finish the run when the gameplay song ends and transition to `PowerUp`

---

### **Immediate Gameplay Priorities**
1. **Guardrail Reliability**
   - **Objective**: Make the outer-planet boundary hold even when runtime
     surface generation lags.
   - **Validation**:
     - The player cannot beat the boundary by outrunning tile generation and
       slipping under the planet.
     - Crossing out of the authored core now feels like a world-law
       transition rather than a rendering race.

2. **Open Flight Preservation**
   - **Objective**: Keep the ship free in 6DOF while still respecting the
     planet.
   - **Validation**:
     - The player can still fly around the planet and away from it.
     - The guardrail does not read like a blocker wall.

3. **Platform Efficiency**
   - **Objective**: Keep the solution cheap enough for console/desktop, tablet,
     `iOS`, and `Android`.
   - **Validation**:
     - The hard rule is one active spherical boundary check, not dependence on
       dense mesh generation.
     - Procedural surface visuals stay low-cost and secondary.

4. **Loop Preservation**
   - **Objective**: Do not break flyer spawning, goal saving, HUD readability,
     or song-ended transition behavior.

---

### **Current Runtime Truth**
- `APlanetBody` owns `PlanetCenter`, `PlanetRadius`, clearance, and authored
  core bounds built from relevant non-sky level actors.
- `ASpaceshipPawn` now selects the nearest valid `APlanetBody` and enforces the
  guardrail against that authority instead of talking directly to the runtime
  surface generator.
- The authored `Level-1` core is explicitly exempt from the planet barrier, so
  static gameplay geometry such as the goal path remains authoritative inside
  that core.
- `APlanetSurfaceRuntime` now uses `APlanetBody` for planet math and stays a
  visual companion only.
- Current default authority values:
  - `PlanetRadius = 1,500,000`
  - `GuardrailClearance = 350`
- Current default visual values:
  - `TileWorldSize = 30,000`
  - `TileResolution = 8`
  - `TileRingCount = 2`
  - `RebuildDistance = 12,000`

---

### **Required Reading**
- `README.md`
- `Docs/AGENT_BOOTSTRAP.md`
- `Docs/HANDOFF.md`
- `Docs/level1-landscape-boundary.md`
- `Docs/level1-media-playback.md`
- `Source/Zhoenus/PlanetBody.cpp`
- `Source/Zhoenus/PlanetSurfaceRuntime.cpp`
- `Source/Zhoenus/SpaceshipPawn.cpp`
- `Source/Zhoenus/ZhoenusPawn.cpp`
- `Source/Zhoenus/SaveThemAllGameMode.cpp`
- `Source/Zhoenus/DonutFlyerSpawner.cpp`

---

### **Next Steps**
1. Play `Level-1` and confirm the guardrail prevents under-planet travel even
   if surface generation is visibly late.
2. Tune `PlanetRadius`, clearance, and visual tile settings only if the feel or
   handoff is wrong.
3. Keep the model simple enough that multiple future planets still work:
   authority body, nearest-body selection, optional visuals.
4. Preserve `Level-1` -> `PowerUp.umap` transition behavior.

---

### **Notes**
- The earlier landscape-expansion experiments are historical context only.
- The earlier single-actor planet prototype was useful, but it tied the ship to
  one visual runtime actor too directly.
- The current model is intentionally small and energy-efficient:
  - one mathematical boundary authority
  - one cheap nearest-body selection path
  - one optional visual outer-surface generator

## Do Not Drift Into

- blind static landscape widening
- blocker-volume perimeter walls
- tying the ship directly to one visual planet generator again
- heavy procedural terrain complexity that is not needed for gameplay

## Expected Agent Behavior

- Start by restating the `SaveThemAll` flight loop under discussion.
- Name the docs and source files you read.
- Keep planet containment cheap, authoritative, and compatible with multiple
  future planets.
- Verify against the live local project state, not against stale notes.

## Handoff Rule

When a task ends, update this file if the active focus changed.
