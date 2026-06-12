"""Create the Fall Guys candy slab kit: six solid-color MaterialInstanceConstants of
the FriendSlop master (white base texture x Tint). Self-quits.

Run: UnrealEditor.exe T66.uproject -ExecutePythonScript="C:/UE/T66/Scripts/CreateFallGuysKitMaterialsAndExit.py" -unattended -nop4 -nosplash
"""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuysKit/create_fallguys_kit.json"
MASTER = "/Game/Materials/M_FriendSlop_FallGuys"
WHITE = "/Engine/EngineResources/WhiteSquareTexture"
DEST = "/Game/World/Terrain/FallGuysKit"

# Fall Guys reference palette: bright saturated candy, matte read.
KIT = {
    "MI_FallGuys_Floor": (0.16, 0.55, 0.93, 1.0),     # sky blue
    "MI_FallGuys_Wall": (0.93, 0.36, 0.66, 1.0),      # bubblegum pink
    "MI_FallGuys_Ceiling": (0.62, 0.52, 0.92, 1.0),   # soft lavender
    "MI_FallGuys_Platform": (0.98, 0.78, 0.15, 1.0),  # sunny yellow
    "MI_FallGuys_Ramp": (0.78, 0.32, 0.90, 1.0),      # magenta-purple
    "MI_FallGuys_Mesa": (0.42, 0.32, 0.95, 1.0),      # violet
}

out = {"materials": [], "errors": []}
MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary
AT = unreal.AssetToolsHelpers.get_asset_tools()

try:
    master = EAL.load_asset(MASTER)
    white = EAL.load_asset(WHITE)
    if not master:
        out["errors"].append(f"master missing: {MASTER}")
    else:
        for name, rgba in KIT.items():
            mi_path = f"{DEST}/{name}"
            if EAL.does_asset_exist(mi_path):
                EAL.delete_asset(mi_path)
            mi = AT.create_asset(name, DEST, unreal.MaterialInstanceConstant,
                                 unreal.MaterialInstanceConstantFactoryNew())
            if not mi:
                out["errors"].append(f"create failed: {name}")
                continue
            MEL.set_material_instance_parent(mi, master)
            if white:
                MEL.set_material_instance_texture_parameter_value(mi, "BaseColorTexture", white)
            MEL.set_material_instance_vector_parameter_value(
                mi, "Tint", unreal.LinearColor(rgba[0], rgba[1], rgba[2], rgba[3]))
            EAL.save_asset(mi_path, only_if_is_dirty=False)
            out["materials"].append(mi_path)
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[FallGuysKit] " + json.dumps(out))
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception:
        pass
