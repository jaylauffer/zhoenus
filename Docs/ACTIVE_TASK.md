# Zhoenus Active Task

Update this file as priorities change.

## **ACTIVE_TASK.md**  
**Revised 2026-04-25**  
**Last Updated by: codex**  

### **Current Focus**  
Extend `Level-1` with flat, low-cost landscape so the current terrain edge does
not admit under-landscape flight during normal play. This fix should preserve
the open 6DOF feel and stay realistic for `iOS` and `Android`.

---

### **Immediate Gameplay Priorities**  
1. **Reachable Seam Survey**  
   - **Objective**: Confirm exactly which reachable edge or seam in `Level-1`
     lets the player reach under-landscape space during normal play.  
   - **Validation**:  
     - The escape route is reproduced and described clearly.  
     - The first terrain-extension target is limited to the seam that is
       actually reachable in normal play.  
     - The task stays framed as a landscape-expansion pass rather than a pawn
       recovery or blocker-volume redesign.  

2. **Extension Rule**  
   - **Objective**: Choose a terrain-extension behavior that fits the 6DOF
     flight game and mobile budget.  
   - **Validation**:  
     - The reachable landscape seam is pushed far enough away that normal play
       does not reach under-terrain space.  
     - The outer terrain stays visually and technically cheaper than the
       gameplay core.  
     - Normal flight space still feels open and readable.  

3. **Loop Preservation**  
   - **Objective**: Fix the boundary without damaging the existing
     `SaveThemAll` loop.  
   - **Validation**:  
     - Gathering and saving `DonutFlyers` still works normally.  
     - Reticle and HUD readability are not made worse by the containment fix.  
     - `Level-1` still transitions to `PowerUp.umap` when the gameplay song
       ends.  

---

### **Validation Criteria**  
- **Containment**:  
  - The player cannot readily reach and remain beneath the landscape during
    normal play.  
  - The expanded outer terrain does not create new obvious seams or traversal
    traps.  
- **Gameplay Integrity**:  
  - Normal 6DOF flight in the intended arena still feels good.  
  - The save loop remains readable and playable.  
  - Existing `Level-1` to `PowerUp` song-ended transition behavior still works.  
- **Platform Cost**:
  - The solution remains reasonable for the project's `iOS` and `Android`
    targets.  
  - The outer landscape band is kept flat and low cost instead of turning
    `Level-1` into a giant high-detail terrain.

---

### **Required Reading**  
- `README.md` (current project state and follow-ups)  
- `Docs/level1-landscape-boundary.md` (documented current problem and evidence)  
- `Docs/AGENT_BOOTSTRAP.md` (project gameplay guardrails)  
- `Docs/HANDOFF.md` (current cross-task context)  
- `Docs/level1-media-playback.md` (other active `Level-1` runtime ownership)  
- `Source/Zhoenus/SpaceshipPawn.cpp` (current player movement and collision behavior)  
- `Source/Zhoenus/SaveThemAllGameMode.cpp` (gameplay map ownership and transition rules)  
- `Source/Zhoenus/DonutFlyerSpawner.cpp` (current landscape-above-ground rule at spawn time)  
- `Content/Map/Level-1.umap` (map-side escape path and containment state)  

---

### **Next Steps**  
1. **Landscape Inspection**: Inspect the current `Level-1` landscape layout and determine the cheapest flat extension pattern.  
2. **Outer Band Iteration**: Add a first low-detail terrain band around the reachable edge instead of using perimeter blockers.  
3. **Cost Check**: Keep the added terrain conservative enough for mobile targets.  
4. **Validation**: Verify the seam is materially harder to reach without making the intended flight arena feel cramped or arbitrary.  

---

### **Notes**  
- **Landscape edge first**: This currently reads as a reachable terrain-edge
  problem before it reads as a ship-handling bug.  
- **Current runtime truth**: `ASpaceshipPawn` has free 6DOF movement with
  sweep-based collision, but no explicit altitude clamp or out-of-bounds
  recovery path was found.  
- **Landscape rule gap**: The project already treats the landscape like ground
  for `DonutFlyer` spawn validation, but that rule does not currently constrain
  player flight.  
- **Chosen direction**: Prefer map-side flat low-cost landscape extension over
  perimeter blockers or a runtime under-terrain recovery redesign.  
- **Mobile guardrail**: Do not respond to this by making `Level-1` a giant
  fully detailed landscape. The added terrain should be cheap.  
- **Gameplay transition rule**: `Level-1.umap` should still transition to
  `PowerUp.umap` when the gameplay song ends. This boundary task should not
  disturb that existing flow.  


## Do Not Drift Into

- generic engine cleanup with no gameplay payoff
- speculative architecture work
- UI polish that ignores the 6DOF save loop
- feature creep unrelated to flying, saving, points, or power-up flow

## Expected Agent Behavior

- Start by restating the gameplay behavior under discussion.
- Name the docs and source files you read.
- Keep fixes small, testable, and player-facing.
- If you are not improving the save loop, explain why the work is still worth doing now.

## Handoff Rule

When a task ends, update this file if the active focus changed.
