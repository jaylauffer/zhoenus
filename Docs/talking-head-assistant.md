# Level-1 Talking Head Assistant

Date: 2026-04-27

## Purpose

`Level-1` has a named in-world anchor actor, `TalkingHeadBase`, for Jay's local
agent presence.

The runtime goal is:

- show Jay's assistant image above the shiny metallic base
- keep the assistant in the `SaveThemAll` fiction
- let players ask natural-language questions
- answer with useful flyer, gate, battery, and run-status guidance
- keep the game playable when no local model is running

This is not a diffusion task. A diffusion model may eventually produce or
animate face assets, but the first implementation is a deterministic game actor
that can display an authored face texture and expose a clean agent seam.

## Runtime Shape

`ASaveThemAllGameMode` auto-spawns `AZhoenusTalkingHeadAssistant` over the first
actor it can resolve by:

- actor tag `TalkingHeadBase`
- actor object name `TalkingHeadBase`
- actor object name beginning with `TalkingHeadBase_`
- editor actor label `TalkingHeadBase`

The assistant uses:

- a billboard face projection
- a caption line
- a light pulse while speaking
- a pawn overlap sphere for future interaction prompts
- game-state-aware fallback hints

Current fallback hints read:

- `ASaveThemAllGameState::Saved`
- `ASaveThemAllGameState::Total`
- the player's current follower count

## Natural Language Seam

The first native seam is:

- `SubmitPlayerUtterance(FText PlayerUtterance)`
- `OnPlayerUtterance`
- `SetAssistantLine(FText NewLine, bool bNewSpeaking)`

`SubmitPlayerUtterance` shows the built-in reply at once, then asks the local
model: loadngo's `gpt_oss_serve` (loadngo `docs/GPT_OSS.md`, "Serving"), one JSON
line per call on its Unix socket, `~/.loadngo/node/gpt-oss.sock`. No HTTP and no
token: the socket's directory is only the user's. The call runs on a pool thread
(`ZhoenusModelCall::Ask`, blocked in the kernel until the reply), and the answer
replaces the caption on the game thread and stays `ModelReplyHoldSeconds` before
gameplay hints resume. A newer question drops an older reply.

The model stays optional. With no server, a busy one or a late reply, the
built-in reply stays and the log says why (`LogZhoenusTalkingHead`). Nothing
waits on the model in flight, scoring or the power-up loop.

How the player asks is not built yet. For now:

- `Zhoenus.Ask <question>` in the console;
- `-ZhoenusAsk="<question>"` on the command line asks once the head is up
  (demos and tests);
- `-ZhoenusModelSocket=<path>` points at another socket.

Start the model with `cargo run --release -p loadngo-gpt-oss --bin gpt_oss_serve`
in `~/pudding/loadngo`.

## Config

The relevant defaults live under:

- `[/Script/Zhoenus.SaveThemAllGameMode]`
- `[/Script/Zhoenus.ZhoenusTalkingHeadAssistant]`

Important fields:

- `bSpawnTalkingHeadAssistant`
- `TalkingHeadBaseActorName`
- `TalkingHeadHeightAboveBase`
- `TalkingHeadSpawnOffset`
- `FaceTexturePath`
- `HintRefreshSeconds`
- `bAutoUpdateGameplayHints`
- `bAskLocalModel`, `ModelSocketPath`, `ModelDeadlineMs`, `ModelReplyHoldSeconds`

## Guardrails

- The assistant helps the player save flyers; it is not wallet UI.
- Do not connect Zhoenus directly to qcoin from this actor.
- Do not make a local AI server mandatory for run completion.
- Do not ship a trusted EAB token in this path.
- If voice input is added, treat it as local-first and optional.
