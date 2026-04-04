# Goose Workflow

## Session Entry Point

Start Goose for this repo with:

```bash
Script/run_goose_zhoenus.sh
```

That wrapper rebuilds:

- `Saved/.goose_context.md`

from:

- `Docs/AGENT_BOOTSTRAP.md`
- `Docs/ACTIVE_TASK.md`
- `Docs/HANDOFF.md`

## Important Behavior

Goose does not automatically absorb doc changes into an already-running session.

If you update:

- `Docs/AGENT_BOOTSTRAP.md`
- `Docs/ACTIVE_TASK.md`
- `Docs/HANDOFF.md`
- or any new doc Goose should use, such as `Docs/level1-media-playback.md`

restart the Goose session or re-run the wrapper so `Saved/.goose_context.md`
gets rebuilt first.

## Current Media-System Note

For `Level-1` media playback tasks, Goose should read:

- `Docs/level1-media-playback.md`
- `Source/Zhoenus/LevelVideoSurfaceManager.cpp`
- `Source/Zhoenus/SaveThemAllGameMode.cpp`
