# DonutFlyer Spawn Analysis

Date: 2026-03-31

## Question

Do DonutFlyers respawn during `SaveThemAllGameMode`, or are they all created at level start?

## Current Answer

There is no runtime respawn path in the current code.

- `ADonutFlyerSpawner::BeginPlay()` is the place where DonutFlyers are created.
- `ASaveThemAllGameMode` does not recreate flyers after they are destroyed.
- No timer, destroy callback, or game-state refill path was found that spawns replacement donuts during play.

## Why It Could Feel Like They Refill

- The original spawner counted `SpawnAmount` as the expected total, but the game mode did not verify how many donuts actually made it into the world.
- The original landscape check only proved there was terrain somewhere below the random point; it did not explicitly reject spawn points that were too close to or below the terrain surface.
- Because of that, the requested count and the live count could drift apart in perception even without any real respawn system.

## Implemented Change

- The spawner now rejects random candidates unless they are at least `MinSpawnHeightAboveGround` above the terrain hit at that XY position.
- The spawner now caps its placement attempts and logs requested count, actual spawned count, and total attempts.
- `ASaveThemAllGameMode` now counts actual live `ADonutFlyerPawn` actors on the next tick after startup and stores that number in `ASaveThemAllGameState::Total`.
- The game mode also logs requested versus actual startup population so the spawn result is visible in logs.

## Resulting Interpretation

- If flyers appear to "show up later," that is not because the game is intentionally refilling destroyed donuts.
- The more likely explanation is that the initial population was distributed broadly, hard to see, or previously not counted accurately.
- With the new logging and actual-count tracking, manual testing should now show whether the startup population really matches the requested count.
