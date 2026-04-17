**Zhoenus Handoff - 2026-04-05**

### Current Repair
AdjustShipUI label restoration via blueprint path (no gameplay changes)

### Required Reading
- `Docs/gamepad-controls-analysis.md` (controls and state transitions)
- `Docs/point-system-and-convert.md` (stat labels and UI alignment)
- `Source/Zhoenus/AdjustShipUI.cpp`, `PowerUpStatWidgetUI.cpp` (label restoration logic)

### Current Focus
- Reticle Refinement: Ensure visual feedback supports 6DOF flight and goal alignment
- Button Highlight Usability: Improve visibility on Startup/AdjustShipUI screens without distracting from flight mechanics
- Music Routing: Keep gameplay tracks and menu tracks separate so menu screens
  use lobby music while `Level-1` keeps the run-ending gameplay playlist model

### Music Routing Rule
- `Music/Game/*` is source material for the gameplay playlist
- `Music/Lobby/*` is source material for the lobby/menu playlist
- `Startup.umap` and `PowerUp.umap` should be treated as lobby/menu context for
  music selection
- `Level-1.umap` remains gameplay context
- Current intended lobby track: `Music/Lobby/Tropical-Delight-Menus-BGM.wav`

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
- Saving all DonutFlyers should eventually produce an EAB achievement claim, but it should not replace the song-ended transition to PowerUp.umap
