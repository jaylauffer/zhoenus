#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BOOTSTRAP_FILE="${GOOSE_BOOTSTRAP_FILE:-$ROOT/Docs/AGENT_BOOTSTRAP.md}"
ACTIVE_TASK_FILE="${GOOSE_ACTIVE_TASK_FILE:-$ROOT/Docs/ACTIVE_TASK.md}"
HANDOFF_FILE="${GOOSE_HANDOFF_FILE:-$ROOT/Docs/HANDOFF.md}"
COMBINED_FILE="${GOOSE_MOIM_MESSAGE_FILE:-$ROOT/Saved/.goose_context.md}"
SESSION_NAME="${GOOSE_SESSION_NAME:-zhoenus}"

export PATH="$HOME/.cargo/bin:/opt/homebrew/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:${PATH:-}"

cd "$ROOT"

if [[ ! -f "$BOOTSTRAP_FILE" ]]; then
  echo "Missing Goose bootstrap file: $BOOTSTRAP_FILE" >&2
  exit 1
fi

if ! command -v goose >/dev/null 2>&1; then
  echo "goose is not installed or not on PATH" >&2
  exit 1
fi

mkdir -p "$ROOT/Saved"
{
  cat "$BOOTSTRAP_FILE"
  if [[ -f "$ACTIVE_TASK_FILE" ]]; then
    printf '\n\n---\n\n'
    cat "$ACTIVE_TASK_FILE"
  fi
  if [[ -f "$HANDOFF_FILE" ]]; then
    printf '\n\n---\n\n'
    cat "$HANDOFF_FILE"
  fi
} >"$COMBINED_FILE"

export GOOSE_MOIM_MESSAGE_FILE="$COMBINED_FILE"

if [[ $# -eq 0 ]]; then
  exec goose session --name "$SESSION_NAME"
fi

exec goose "$@"
