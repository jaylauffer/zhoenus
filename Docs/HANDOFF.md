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

### State Transition Rules
| From Map       | To Map         | Trigger                          | Notes                                  |
|----------------|----------------|----------------------------------|----------------------------------------|
| `Level-1.umap` | `Startup.umap` | Song finishes playing            | Session ends; player returns to menu   |
| `Level-1.umap` | `PowerUp.umap` | DonutFlyers all saved            | Transition to post-flight UI           |
| `PowerUp.umap` | `Level-1.umap` | "Again" button clicked          | Start new gameplay session             |
| `PowerUp.umap` | `Startup.umap` | "Back" button clicked           | Return to main menu                    |
| `Startup.umap` | `Level-1.umap` | "Play" button clicked           | Launch core flight loop                |
| `Startup.umap` | (Exit)         | "Enough" button clicked         | Close app, no further UI interaction   |

### Validation Criteria
- Confirm song-ending condition triggers transition to Startup.umap
- Ensure "Again" and "Back" buttons drive correct state transitions
- Verify no UI elements interfere with flight mechanics in Level-1.umap