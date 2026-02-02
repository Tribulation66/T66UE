"""
Debug helper: print which ground material is configured/loaded.

Run (PowerShell):
& "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "C:/UE/T66/T66.uproject" -run=pythonscript -script="C:/UE/T66/Scripts/DebugGroundMaterial.py" -unattended -nop4 -nosplash -nullrhi
"""

from __future__ import annotations

import unreal


def _print_soft(obj, prop_name: str):
    try:
        v = obj.get_editor_property(prop_name)
    except Exception as e:
        unreal.log_warning(f"[GroundDebug] {obj}: no property {prop_name} ({e})")
        return

    # TSoftObjectPtr often prints as a SoftObjectPath; be defensive.
    try:
        s = str(v)
    except Exception:
        s = "<unprintable>"

    resolved = None
    try:
        # If v is a soft object path/pointer, try resolve via load_asset (won't load if already resident?).
        if hasattr(v, "to_soft_object_path"):
            sop = v.to_soft_object_path()
            resolved = sop.try_load()
        elif isinstance(v, unreal.SoftObjectPath):
            resolved = v.try_load()
    except Exception:
        resolved = None

    unreal.log(f"[GroundDebug] {obj.get_name()}.{prop_name} = {s} | loaded={resolved is not None}")


def main():
    unreal.log("=== Ground material debug: START ===")

    # Native CDO
    try:
        gm_cls = unreal.find_class("T66GameMode")
        if gm_cls:
            cdo = unreal.get_default_object(gm_cls)
            _print_soft(cdo, "GroundFloorMaterial")
            try:
                unreal.log(f"[GroundDebug] Native bAutoSetupLevel={cdo.get_editor_property('bAutoSetupLevel')}")
            except Exception:
                pass
    except Exception as e:
        unreal.log_warning(f"[GroundDebug] Failed to inspect native CDO: {e}")

    # Blueprint GameMode
    bp_path = "/Game/Blueprints/GameModes/BP_GameplayGameMode.BP_GameplayGameMode"
    bp = unreal.EditorAssetLibrary.load_asset(bp_path)
    if bp:
        try:
            gen = bp.generated_class
            cdo2 = unreal.get_default_object(gen)
            unreal.log(f"[GroundDebug] Loaded BP: {bp_path}")
            _print_soft(cdo2, "GroundFloorMaterial")
            try:
                unreal.log(f"[GroundDebug] BP bAutoSetupLevel={cdo2.get_editor_property('bAutoSetupLevel')}")
            except Exception:
                pass
        except Exception as e:
            unreal.log_warning(f"[GroundDebug] Failed to inspect BP CDO: {e}")
    else:
        unreal.log_warning(f"[GroundDebug] Missing BP asset: {bp_path}")

    unreal.log("=== Ground material debug: DONE ===")


if __name__ == "__main__":
    main()

