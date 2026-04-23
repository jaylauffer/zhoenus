**Zhoenus Handoff - 2026-04-05**

### Current Repair
AdjustShipUI label restoration via blueprint path (no gameplay changes)

### Required Reading
- `Docs/gamepad-controls-analysis.md` (controls and state transitions)
- `Docs/point-system-and-convert.md` (stat labels and UI alignment)
- `Docs/lobby-music-design.md` (menu-context music ownership and behavior)
- `Source/Zhoenus/AdjustShipUI.cpp`, `PowerUpStatWidgetUI.cpp` (label restoration logic)

### Current Focus
- Reticle Refinement: Ensure visual feedback supports 6DOF flight and goal alignment
- Menu Contrast Accessibility: Improve `PowerUp`, `Convert`, and `AdjustShip` contrast so the menus are easier to read for more players, but keep this below reticle work in priority
- Button Highlight Usability: Improve visibility on Startup/AdjustShipUI screens without distracting from flight mechanics
- Music Routing: Keep gameplay tracks and menu tracks separate so menu screens
  use lobby music while `Level-1` keeps the run-ending gameplay playlist model

### Process Hygiene
- If you launch `UnrealEditor`, `UnrealEditor-Cmd`, or `-game` runtime sessions during validation, shut them down before ending the turn.
- Do not leave menu/game preview processes running in the background for the user unless they explicitly ask for that.
- After Unreal builds, check for lingering local `.NET` `MSBuild.dll /nodemode:1` worker processes and terminate them if they were only spawned for the build you ran.

### Music Routing Rule
- `Music/Game/*` is source material for the gameplay playlist
- `Music/Lobby/*` is source material for the lobby/menu playlist
- `Startup.umap` and `PowerUp.umap` should be treated as lobby/menu context for
  music selection
- `Level-1.umap` remains gameplay context
- Current curated lobby runtime track:
  `/Game/Sound/Lobby/LobbySong.LobbySong`
- Lobby music should be owned by `AZhoenusLobbyGameMode`
- Lobby/menu maps should use `AZhoenusLobbyPlayerController`, not the gameplay
  `AZhoenusPlayerController`, so flight touch UI never appears over menu screens
- `Level-*` maps should route back to `SaveThemAllV1`

### State Transition Rules
| From Map       | To Map         | Trigger                          | Notes                                  |
|----------------|----------------|----------------------------------|----------------------------------------|
| `Level-1.umap` | `PowerUp.umap` | Song finishes playing            | Session ends; player enters post-flight UI |
| `PowerUp.umap` | `Level-1.umap` | "Again" button clicked          | Start new gameplay session             |
| `PowerUp.umap` | `Startup.umap` | "Back" button clicked           | Return to main menu                    |
| `Startup.umap` | `Level-1.umap` | "Play" button clicked           | Launch core flight loop                |
| `Startup.umap` | (Exit)         | "Enough" button clicked         | Close app, no further UI interaction   |

### Validation Criteria
- Confirm song-ending condition triggers transition to PowerUp.umap
- Ensure "Again" and "Back" buttons drive correct state transitions
- Verify no UI elements interfere with flight mechanics in Level-1.umap
- Confirm menu screens use the lobby playlist instead of the gameplay playlist
- Confirm lobby music fades in around `42%` volume and replays after a random
  `16`-`42` second silence interval
- Saving all DonutFlyers should eventually produce an EAB achievement claim, but it should not replace the song-ended transition to PowerUp.umap
