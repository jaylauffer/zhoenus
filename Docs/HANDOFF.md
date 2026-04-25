**Zhoenus Handoff - 2026-04-25**

### Current Repair
Level-1 landscape boundary documentation and first-pass terrain-extension
planning

### Required Reading
- `README.md` (repo state and current follow-ups)
- `Docs/AGENT_BOOTSTRAP.md` (gameplay and workflow guardrails)
- `Docs/ACTIVE_TASK.md` (active Level-1 boundary priority)
- `Docs/level1-landscape-boundary.md` (current problem statement and chosen
  direction)
- `Docs/level1-media-playback.md` (other `Level-1` runtime ownership that
  should not be broken by map work)
- `Source/Zhoenus/SpaceshipPawn.cpp` (current 6DOF movement and collision
  behavior)
- `Source/Zhoenus/SaveThemAllGameMode.cpp` (song-ended run completion and map
  transition behavior)
- `Source/Zhoenus/DonutFlyerSpawner.cpp` (current landscape-above-ground rule
  for flyer spawning)
- `Content/Map/Level-1.umap` (landscape layout and reachable seam)

### Current Focus
- Level-1 Boundary: Extend the reachable `Level-1` landscape outward with flat,
  low-cost terrain so the current seam is not reachable during normal play.
- Flight Feel Protection: Do not solve this with perimeter blocker walls that
  make the open 6DOF space feel boxed in.
- Mobile Cost Guardrail: Keep the added outer band materially cheaper than the
  authored gameplay core so the map remains plausible for `iOS` and `Android`.
- Loop Preservation: Do not damage flyer gathering, goal saving, HUD
  readability, or the song-ended transition to `PowerUp.umap`.
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
- Confirm the reachable terrain seam is pushed far enough away that under-
  landscape flight is no longer part of normal play
- Ensure the outer terrain band stays flat and low cost rather than turning
  Level-1 into a giant detailed landscape
- Verify no new blocker-wall feel is introduced into normal flight
- Confirm song-ending condition triggers transition to PowerUp.umap
- Ensure "Again" and "Back" buttons drive correct state transitions
- Verify no UI elements interfere with flight mechanics in Level-1.umap
- Confirm menu screens use the lobby playlist instead of the gameplay playlist
- Confirm lobby music fades in around `42%` volume and replays after a random
  `16`-`42` second silence interval
- Saving all DonutFlyers should eventually produce an EAB achievement claim, but it should not replace the song-ended transition to PowerUp.umap
