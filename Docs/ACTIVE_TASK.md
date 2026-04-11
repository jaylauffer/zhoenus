# Zhoenus Active Task

Update this file as priorities change.

## **ACTIVE_TASK.md**  
**Revised 2026-04-05**  
**Last Updated by: goose**  

### **Current Focus**  
Refine **reticle feedback** and **button highlight usability** to ensure the 6DOF flight/save loop remains unobstructed, while maintaining clarity on menu interactions and session flow.  

---

### **Immediate Gameplay Priorities**  
1. **Reticle Refinement**  
   - **Objective**: Ensure the reticle provides **clear, intuitive feedback** for 6DOF movement and goal alignment.  
   - **Validation**:  
     - Reticle changes (e.g., size, color) must be **synced with power-up screen interactions**.  
     - Test edge cases (e.g., reticle behavior when DonutFlyers are near the goal edge).  

2. **Button Highlight Usability**  
   - **Objective**: Improve visibility of **Startup.umap** and **PowerUp.umap** button highlights without distracting from flight mechanics.  
   - **Validation**:  
     - No flashing or loud feedback during flight (6DOF mechanics take priority).  
     - "Again" and "Back" buttons on `PowerUp.umap` must drive **expected state transitions**.  

3. **Song-Ending Condition**  
   - **Objective**: Validate that gameplay sessions **end when the song finishes** and then advance to `PowerUp.umap`, regardless of how many DonutFlyers were saved.  
   - **Validation**:  
     - Confirm transitions to `PowerUp.umap` after song ends.  

---

### **Validation Criteria**  
- **Reticle Behavior**:  
  - Must align with `Docs/gamepad-controls-analysis.md` for UI feedback consistency.  
  - No visual/auditory feedback during flight (only active on menu screens).  
- **Button Highlights**:  
  - Must be **visible but non-intrusive** on `Startup.umap` and `PowerUp.umap`.  
  - No UI elements should interfere with the core flight/save loop.  
- **State Transitions**:  
  - "Again" button must transition to `Level-1.umap` for a new session.  
  - "Back" button must return to `Startup.umap`.  
  - Song-ending condition must trigger automatic transition to `PowerUp.umap`.  

---

### **Required Reading**  
- `Docs/gamepad-controls-analysis.md` (reticle behavior and UI feedback)  
- `Docs/point-system-and-convert.md` (stat label mappings and UI alignment)  
- `Source/Zhoenus/SpaceshipPawn.cpp`, `Goal.cpp` (reticle logic)  
- `PowerUpScreenUI.cpp`, `PowerUpRootUI.cpp` (button highlight mechanics)  
- `SaveThemAllGameMode.cpp` (state transition logic)  

---

### **Next Steps**  
1. **Reticle Audit**: Review `SpaceshipPawn.cpp` and `Goal.cpp` for reticle behavior.  
2. **Highlight Review**: Analyze `PowerUpScreenUI.cpp` for button highlight logic.  
3. **State Transition Validation**: Confirm "Again"/"Back" buttons and song-ending behavior in `SaveThemAllGameMode.cpp`.  
4. **Stability Testing**: Run extended playtests to ensure no new friction points emerge.  

---

### **Notes**  
- **No UI distractions during flight**: Button highlights and power-up screens are **inactive in Level-1.umap**.  
- **Song-ending condition**: Sessions end **automatically when the song finishes**, not when all DonutFlyers are saved.  
- **Full-save behavior**: Saving every DonutFlyer should eventually generate an EAB achievement claim, but it should not short-circuit the run or replace the song-ended transition to `PowerUp.umap`.  
- **Map Transitions**:  
  - `Level-1.umap` → `PowerUp.umap` (song ends; end of gameplay session)  
  - `PowerUp.umap` → `Level-1.umap` ("Again" button)  
  - `PowerUp.umap` → `Startup.umap` ("Back" button)  


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
