#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
EXCLUDES_FILE="${ROOT}/Script/portable-copy-excludes.txt"

usage() {
  cat <<'EOF'
Usage: Script/sync_portable_project.sh [--dry-run] [--delete] [--list-excludes] <destination> [extra rsync args...]

Copies a rebuildable Zhoenus project tree without Unreal editor cache or Apple-only build baggage.

Examples:
  Script/sync_portable_project.sh /Volumes/Hercules/ZhoenusPortable
  Script/sync_portable_project.sh --dry-run /Volumes/Hercules/ZhoenusPortable --stats
  Script/sync_portable_project.sh /Volumes/Hercules/ZhoenusPortable --exclude='.git/'
EOF
}

dry_run=0
delete_mode=0
destination=""
extra_args=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --dry-run)
      dry_run=1
      shift
      ;;
    --delete)
      delete_mode=1
      shift
      ;;
    --list-excludes)
      cat "$EXCLUDES_FILE"
      exit 0
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    --)
      shift
      extra_args+=("$@")
      break
      ;;
    -*)
      extra_args+=("$1")
      shift
      ;;
    *)
      destination="$1"
      shift
      extra_args+=("$@")
      break
      ;;
  esac
done

if [[ -z "$destination" ]]; then
  usage >&2
  exit 1
fi

if [[ ! -f "$EXCLUDES_FILE" ]]; then
  echo "Missing exclude file: $EXCLUDES_FILE" >&2
  exit 1
fi

mkdir -p "$destination"

rsync_args=(
  -a
  --exclude-from="$EXCLUDES_FILE"
)

if [[ $dry_run -eq 1 ]]; then
  rsync_args+=(-n)
else
  rsync_args+=(--progress)
fi

if [[ $delete_mode -eq 1 ]]; then
  rsync_args+=(--delete)
fi

if [[ ${#extra_args[@]} -gt 0 ]]; then
  rsync_args+=("${extra_args[@]}")
fi

exec rsync "${rsync_args[@]}" "${ROOT}/" "${destination}/"
