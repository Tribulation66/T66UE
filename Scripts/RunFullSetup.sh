#!/usr/bin/env bash
# Full T66 project setup via Unreal Editor Python.
# 1) CreateAssets.py - DataTables (Heroes, Companions, Items), levels.
# 2) FullSetup.py - Button widget, Game Instance, Player Controller, GameModes, CSV import, verify.
#
# Set UE_ROOT to your engine install if not in a default location, e.g.:
#   export UE_ROOT="/Users/Shared/Epic Games/UE_5.7"   # macOS
#   export UE_ROOT="$HOME/UnrealEngine"                 # Linux
#   export UE_ROOT="C:/Program Files/Epic Games/UE_5.7" # Windows (Git Bash)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
UPROJECT="$PROJECT_DIR/T66.uproject"

# Default UE_ROOT by platform
UNAME="$(uname -s)"
if [[ -z "${UE_ROOT:-}" ]]; then
  case "$UNAME" in
    Darwin*)  UE_ROOT="/Users/Shared/Epic Games/UE_5.7" ;;
    MINGW*|MSYS*|CYGWIN*) UE_ROOT="C:/Program Files/Epic Games/UE_5.7" ;;
    *)        UE_ROOT="${HOME}/UnrealEngine" ;;
  esac
fi

# Editor executable by platform
case "$UNAME" in
  Darwin*)
    EDITOR_CMD="$UE_ROOT/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor"
    ;;
  MINGW*|MSYS*|CYGWIN*)
    EDITOR_CMD="$UE_ROOT/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
    ;;
  *)
    EDITOR_CMD="$UE_ROOT/Engine/Binaries/Linux/UnrealEditor"
    ;;
esac

if [[ ! -f "$EDITOR_CMD" ]]; then
  echo "Unreal editor not found at: $EDITOR_CMD"
  echo "Set UE_ROOT to your engine install, e.g. export UE_ROOT=/path/to/UE_5.7"
  exit 1
fi

if [[ ! -f "$UPROJECT" ]]; then
  echo "Project not found: $UPROJECT"
  exit 1
fi

# On Windows (Git Bash/MSYS), pass Windows paths to the editor
if [[ "$UNAME" == MINGW* || "$UNAME" == MSYS* || "$UNAME" == CYGWIN* ]]; then
  if command -v cygpath &>/dev/null; then
    SCRIPT_DIR_WIN="$(cygpath -w "$SCRIPT_DIR")"
    UPROJECT_WIN="$(cygpath -w "$UPROJECT")"
  else
    SCRIPT_DIR_WIN="$SCRIPT_DIR"
    UPROJECT_WIN="$UPROJECT"
  fi
  SCRIPT_ARG1="$SCRIPT_DIR_WIN/CreateAssets.py"
  SCRIPT_ARG2="$SCRIPT_DIR_WIN/FullSetup.py"
  UPROJECT_ARG="$UPROJECT_WIN"
else
  SCRIPT_ARG1="$SCRIPT_DIR/CreateAssets.py"
  SCRIPT_ARG2="$SCRIPT_DIR/FullSetup.py"
  UPROJECT_ARG="$UPROJECT"
fi

echo "=== Full project setup (CreateAssets + FullSetup) ==="
echo "Step 1/2: CreateAssets..."
"$EDITOR_CMD" "$UPROJECT_ARG" -run=pythonscript -script="$SCRIPT_ARG1"
echo "Step 2/2: FullSetup..."
"$EDITOR_CMD" "$UPROJECT_ARG" -run=pythonscript -script="$SCRIPT_ARG2"
echo "=== Full project setup complete ==="
