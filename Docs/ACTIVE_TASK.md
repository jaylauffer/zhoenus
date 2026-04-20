# Zhoenus Active Task

Update this file as priorities change.

## **ACTIVE_TASK.md**  
**Revised 2026-04-05**  
**Last Updated by: goose**  

### **Current Focus**  
Route music intentionally between gameplay and menu contexts while preserving the
existing 6DOF flight/save loop and song-ended transition behavior.

---

### **Immediate Gameplay Priorities**  
1. **Music Playlist Split**  
   - **Objective**: Make the game honor two distinct music intents: gameplay and lobby.  
   - **Validation**:  
     - `Music/Game` feeds the gameplay playlist.  
     - `Music/Lobby` feeds the lobby playlist.  
     - Menu-facing screens do not accidentally consume gameplay-run tracks.  

2. **Lobby Music Behavior**  
   - **Objective**: Use the lobby playlist as the menu-screen background music path.  
   - **Validation**:  
     - `Startup.umap` uses lobby music.  
     - `PowerUp.umap` uses lobby music.  
     - Current lobby runtime track `/Game/Sound/Lobby/LobbySong.LobbySong`
       is the active menu target until more lobby tracks exist.  
     - Playback starts at about `42%` volume with fade in.  
     - After the song ends, replay waits a random `16`-`42` second silence
       interval.  

3. **Gameplay Song-Ending Condition**  
   - **Objective**: Preserve the rule that gameplay sessions **end when the gameplay song finishes** and then advance to `PowerUp.umap`, regardless of how many DonutFlyers were saved.  
   - **Validation**:  
     - Confirm transitions to `PowerUp.umap` after the gameplay track ends.  

---

### **Validation Criteria**  
- **Playlist Routing**:  
  - `Music/Game` and `Music/Lobby` remain distinct source folders.  
  - Menu screens use the lobby playlist.  
  - `Level-1.umap` uses the gameplay playlist path.  
- **State Transitions**:  
  - "Again" button must transition to `Level-1.umap` for a new session.  
  - "Back" button must return to `Startup.umap`.  
  - Gameplay song-ending condition must trigger automatic transition to `PowerUp.umap`.  

---

### **Required Reading**  
- `Docs/music-source-playlists.md` (game vs lobby playlist intent)  
- `Docs/lobby-music-design.md` (lobby context and playback behavior)  
- `Docs/save-them-all-music-playback.md` (current gameplay playlist path)  
- `Docs/gamepad-controls-analysis.md` (reticle behavior and UI feedback)  
- `Docs/point-system-and-convert.md` (stat label mappings and UI alignment)  
- `PowerUpScreenUI.cpp`, `PowerUpRootUI.cpp` (menu shell context)  
- `SaveThemAllGameMode.cpp` (gameplay music and state transition logic)  

---

### **Next Steps**  
1. **Documented Playlist Intent**: Keep `Music/Game` and `Music/Lobby` rules explicit.  
2. **Menu Music Routing**: Add a lobby playback path for `Startup` and `PowerUp`.  
3. **Gameplay Preservation**: Keep `SaveThemAllGameMode` responsible for the gameplay playlist and song-ended transition.  
4. **Validation**: Verify menu music and gameplay music do not cross-contaminate each other.  

---

### **Notes**  
- **Menu vs gameplay music**: Menu-facing maps should consume lobby music; flight
  remains on the gameplay playlist path.  
- **Lobby mode**: `Startup` and `PowerUp` should use `AZhoenusLobbyGameMode`;
  `Level-*` maps should remain on `SaveThemAllV1`.  
- **Song-ending condition**: Gameplay sessions end **automatically when the gameplay song finishes**, not when all DonutFlyers are saved.  
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
