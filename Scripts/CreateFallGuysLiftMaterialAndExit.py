"""Add MI_FallGuys_Lift to the Fall Guys candy slab kit: mint-green MaterialInstanceConstant
of the FriendSlop master (white base texture x Tint), distinct from the sunny-yellow static
platforms so MOVING surfaces read at a glance. Self-quits.

Run: UnrealEditor.exe T66.uproject -ExecutePythonScript="C:/UE/T66/Scripts/CreateFallGuysLiftMaterialAndExit.py" -unattended -nop4 -nosplash
"""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuysKit/create_fallguys_lift.json"
MASTER = "/Game/Materials/M_FriendSlop_FallGuys"
WHITE = "/Engine/EngineResources/WhiteSquareTexture"
DEST = "/Game/World/Terrain/FallGuysKit"

KIT = {
    "MI_FallGuys_Lift": (0.14, 0.85, 0.55, 1.0),  # mint green (moving lift slabs)
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
