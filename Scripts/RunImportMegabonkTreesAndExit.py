"""
Run ImportMegabonkTrees.py inside Unreal Python.

Do not force QUIT_EDITOR here. The previous shutdown path could crash the UI
editor during LevelEditor/StatusBar teardown even after the import had already
completed successfully.

For unattended runs, prefer UnrealEditor-Cmd.exe so the process exits on its
own after the script finishes.
"""

import os
import sys
import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import ImportMegabonkTrees


def main():
    ImportMegabonkTrees.main()
    unreal.log("[RunImportMegabonkTreesAndExit] Import complete; editor left running intentionally")


if __name__ == "__main__":
    main()
