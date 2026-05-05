#!/usr/bin/env zsh

# ----- compute location -------------------------------------------------
SCRIPT_PATH="$(cd "$(dirname "${(%):-%N}")" \
                && pwd)/$(basename "${(%):-%N}")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"

# ----- example usage ---------------------------------------------------
echo "Containing folder: $SCRIPT_DIR"

../unreal/GenerateProjectFiles.sh -project "$SCRIPT_DIR/zhoenus.uproject" -game -editor

